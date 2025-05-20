#include "flood_fill.hpp"
#include <span>


template <typename T>
__fast_inline constexpr T saturate_cast(const auto & v) {
    using SrcType = std::remove_reference<decltype(v)>::type;
    return static_cast<T>(CLAMP(v, 
        std::numeric_limits<SrcType>::min(), 
        std::numeric_limits<SrcType>::max()));
}


namespace ymd::nvcv2::Shape{


static constexpr std::array<Vector2i,4> OFFSETS_4 = { 
    Vector2i{-1, 0}, 
    Vector2i{0, -1}, 
    Vector2i{0, 1}, 
    Vector2i{1, 0}
};
// const std::array<Vector2u,8> offsets_8 = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},{0, 1}, {1, -1}, {1, 0}, {1, 1} };


template<typename _Tp, class _EqPredicate> int
partition( const std::vector<_Tp>& _vec, std::vector<int>& labels,
          _EqPredicate predicate=_EqPredicate())
{
    int i, j, N = (int)_vec.size();
    const _Tp* vec = &_vec[0];
 
    const int PARENT=0;
    const int RANK=1;
 
    std::vector<int> _nodes(N*2);
    int (*nodes)[2] = (int(*)[2])&_nodes[0];
 
    // The first O(N) pass: create N single-vertex trees
    // nodes[i][PARENT] = -1表示无父节点，所有节点初始化为单独的节点
    for(i = 0; i < N; i++)
    {
        nodes[i][PARENT]=-1;
        nodes[i][RANK] = 0;
    }
 
    // The main O(N^2) pass: merge connected components
    // 每一个节点都和其他所有节点比较，看是否属于同一类
    // 属于同一类的判断 predicate(vec[i], vec[j]),predicate为传入的SimilarRects
    // SimilarRects判断两个矩形框的四个相应顶点的差值的绝对值都在deta范围内，则认为属于同一类，否则是不同类
    // 两层for循环和后面的压缩策略保证了最终形成很多类，每一类以根节点为中心，该类的其余节点的父节点指向根节点
    for( i = 0; i < N; i++ )
    {
        int root = i;
 
        // find root
        // 寻找根节点，每次都是和每个节点对应的根节点比较，如果是单独的节点，根节点就是本身
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
 
        for( j = 0; j < N; j++ )
        {
        	// 同一节点或两个节点的矩形框差距大，则不连接
            if( i == j || !predicate(vec[i], vec[j]))
                continue;
            int root2 = j;
 
			// 寻找可以归为同一类节点的根节点，每次都是和对应的根节点先链接
			// 即比较两个节点的矩形框，连接时，使用两个节点对应的两个根节点
			// 这样保证了已经连接在同一类的不在连接，不同类的也容易连接
            while( nodes[root2][PARENT] >= 0 )
                root2 = nodes[root2][PARENT];
			// 保证已经连接在同一类的不再连接
            if( root2 != root )
            {
                // unite both trees
                // rank表示级别，根节点rank大为0，普通点rank为0，并且根节点的rank随着连接同级根节点的次数增多而增大
                int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
				// root为根节点，root2为单独节点，将root2连接到root上，根节点不变
                if( rank > rank2 )
                    nodes[root2][PARENT] = root;
				// 当root和root2都为根节点，将root连接到root2，并将root2对应的rank加1，root2为根节点，root为单独点，将root连接
				// 到root2上。二者都将根节点更新为root2
                else
                {
                    nodes[root][PARENT] = root2;
                    nodes[root2][RANK] += rank == rank2;
                    root = root2;
                }
				// 根节点的parent必须小于0
                assert(nodes[root][PARENT] < 0 );
 
                int k = j, parent;
 
                // compress the path from node2 to root
                // 下一级节点通过它的根节点连接到上一级根节点时，直接将下一级节点和根节点都连接到上级的根节点
                // 如果是单独的节点连接到某个根节点，循环不改变任何值
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
 
                // compress the path from node to root
                // 同一级节点通过它的根节点连接到同级的根节点，直接将该节点和根节点都连接到同级的根节点，如果是单独
                // 的节点连接到某个根节点，循环不改变任何值
                k = i;
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
            }
        }
    }
 
    // Final O(N) pass: enumerate classes
    labels.resize(N);
	// 总分类数
    int nclasses = 0;
 
    for( i = 0; i < N; i++ )
    {
        int root = i;
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        // re-use the rank as the class label
        // 小于0，则已经统计过
        if( nodes[root][RANK] >= 0 )
            nodes[root][RANK] = ~nclasses++;
		// 每个根节点保存着类别ID的非值，其非值小于0
        labels[i] = ~nodes[root][RANK];
    }
 
    return nclasses;
}


void groupRectangles(std::vector<Rect2u>& rectList, int groupThreshold, real_t eps,
                     std::vector<int>* weights, std::vector<real_t>* levelWeights){
    if( groupThreshold <= 0 || rectList.empty() )
    {
        if( weights )
        {
            size_t i, sz = rectList.size();
            weights->resize(sz);
            for( i = 0; i < sz; i++ )
                (*weights)[i] = 1;
        }
        return;
    }
 
    std::vector<int> labels;
	// 调用partition函数，将所有的矩形框初步分为几类，其中labels为每个矩形框对应的类别编号，
    // eps为判断两个矩形框是否属于同一类的控制参数。
    // 如果两个矩形框的四个相应顶点的差值的绝对值都在deta范围内，则认为属于同一类，否则是不同类。

    // SimilarRects(eps)//TODO
    int nclasses = partition(rectList, labels, SimilarRects(eps));
 
    std::vector<Rect2u> rrects(nclasses);
    std::vector<int> rweights(nclasses, 0);
    std::vector<int> rejectLevels(nclasses, 0);
    std::vector<real_t> rejectWeights(nclasses, std::numeric_limits<real_t>::lowest());
    int i, j, nlabels = (int)labels.size();
    for( i = 0; i < nlabels; i++ )
    {
        int cls = labels[i];
        rrects[cls].x() += rectList[i].x();
        rrects[cls].y() += rectList[i].y();
        rrects[cls].w() += rectList[i].w();
        rrects[cls].h() += rectList[i].h();
        rweights[cls]++;
    }
 
    bool useDefaultWeights = false;
 
    if ( levelWeights && weights && !weights->empty() && !levelWeights->empty() )
    {
        for( i = 0; i < nlabels; i++ )
        {
            int cls = labels[i];
            if( (*weights)[i] > rejectLevels[cls] )
            {
                rejectLevels[cls] = (*weights)[i];
                rejectWeights[cls] = (*levelWeights)[i];
            }
            else if( ( (*weights)[i] == rejectLevels[cls] ) && ( (*levelWeights)[i] > rejectWeights[cls] ) )
                rejectWeights[cls] = (*levelWeights)[i];
        }
    }
    else
        useDefaultWeights = true;
	// 计算每一类别的平均矩形框位置，即每一个类别最终对应一个矩形框
    for( i = 0; i < nclasses; i++ )
    {
        Rect2u r = rrects[i];
        real_t s = real_t(1)/rweights[i];
        rrects[i] = Rect2u(
            saturate_cast<int>(r.x()*s),
            saturate_cast<int>(r.y()*s),
            saturate_cast<int>(r.w()*s),
            saturate_cast<int>(r.h()*s));
    }
 
    rectList.clear();
    if( weights )
        weights->clear();
    if( levelWeights )
        levelWeights->clear();
	// 再次过滤上面分类中得到的所有矩形框
    for( i = 0; i < nclasses; i++ )
    {
        Rect2u r1 = rrects[i];
        int n1 = rweights[i];
        real_t w1 = rejectWeights[i];
        int l1 = rejectLevels[i];
 
        // filter out rectangles which don't have enough similar rectangles
        // 将每一类别中矩形框个数较少的类别过滤掉。
        if( n1 <= groupThreshold )
            continue;
        // filter out small face rectangles inside large rectangles
        // 将嵌在大矩形框内部的小矩形框过滤掉。最后剩下的矩形框为聚类的结果。
        for( j = 0; j < nclasses; j++ )
        {
            int n2 = rweights[j];
 
            if( j == i || n2 <= groupThreshold )
                continue;
            Rect2u r2 = rrects[j];
 
            int dx = saturate_cast<int>( r2.w() * eps );
            int dy = saturate_cast<int>( r2.h() * eps );
 
            if( i != j &&
                r1.x() >= r2.x() - dx &&
                r1.y() >= r2.y() - dy &&
                r1.x() + r1.w() <= r2.x() + r2.w() + dx &&
                r1.y() + r1.h() <= r2.y() + r2.h() + dy &&
                (n2 > std::max(3, n1) || n1 < 3) )
                break;
        }
 
        if( j == nclasses )
        {
            rectList.push_back(r1);
            if( weights )
                weights->push_back(useDefaultWeights ? n1 : l1);
            if( levelWeights )
                levelWeights->push_back(w1);
        }
    }
}
[[maybe_unused]] static real_t iou(const Rect2u & a, const Rect2u & b){

    Rect2u _a = a.abs();
    Rect2u _b = b.abs();

    real_t ins = _a.intersection(_b).get_area();
    if(ins == 0) return 0;

    real_t uni = _a.get_area() + _b.get_area() - ins;

    return (ins / uni);
}


Image<Grayscale> FloodFill::run(const ImageReadable<Binary> & src, const BlobFilter & filter) {
    static constexpr Grayscale labelable = 255;

    const auto size = src.size();
    const auto nrow = size.x;
    const auto ncol = size.y;
    Image<Grayscale> map({nrow, ncol});
    m_blobs.clear();

    for(size_t y = 0u; y < ncol; ++y){
        for(size_t x = 0u; x < nrow; ++x){
            if(src[{x,y}] == Binary::BLACK){
                map[{x,y}] = Grayscale(0);
            }else{
                map[{x,y}] = labelable;
            }
        }
    }

    uint8_t label = 1;

    auto get_neighbor_indices = [&](const size_t row,const size_t col) {
        std::array<Vector2u, 4> result;
        uint8_t i = 0;
        for (const auto& offset : OFFSETS_4) {
            const size_t x = row + offset.x;
            const size_t y = col + offset.y;
            if (x >= 0 && x < nrow && y >= 0 && y < ncol) {
                result[i] = {x, y};
                i++;
            }
        }
        return result;
    };

    // Perform seed filling
    for (size_t row = 0; row < nrow; ++row) {
        for (size_t col = 0; col < ncol; ++col) {
            // Skip background pixels and already labelled pixels
            if (map[{row,col}] != labelable) {
                continue;
            }

            // Perform flood fill starting from (row, col)
            // sstl::vector<Vector2_t<uint8_t>, 256> current_indices;
            std::vector<Vector2_t<uint8_t>> current_indices;
            map[{row,col}] = label;
            Blob blob{
                .rect = Rect2u(Vector2u{row, col}, Vector2u{0,0}),
                .area = 0,
                .index = label,
            };

            for (const auto & neighbor_pos : get_neighbor_indices(row, col)){
                if (map[neighbor_pos] == labelable){
                    map[neighbor_pos] = label;
                    current_indices.push_back(neighbor_pos);

                    blob.area++;
                    blob.rect = blob.rect.merge(neighbor_pos);
                }
            }

            while (!current_indices.empty()) {
                auto current_index = current_indices.back();
                current_indices.pop_back();

                for (const auto& neighbor_pos : get_neighbor_indices(
                    current_index.x, current_index.y)) {
                    if (map[neighbor_pos] == labelable) {
                        map[neighbor_pos] = label;
                        current_indices.push_back(neighbor_pos);

                        blob.area++;
                        blob.rect = blob.rect.merge(neighbor_pos);
                    }
                }
            }

            {
                const auto & rect = blob.rect;
                bool skip_flag = false;

                {
                    skip_flag |= not filter.area_range.has(rect.get_area());
                }

                {
                    skip_flag |= not filter.width_range.has(rect.w());
                    skip_flag |= not filter.height_range.has(rect.h());
                }
                // skip_flag |= rect.h() > 50;
                // skip_flag |= rect.h < 5;
                // skip_flag |= real_t(int(rect)) / blob.area < (1.0 / 0.7);
                // skip_flag |= blob.area < 50;
                // skip_flag |= blob.area > 20;
                // skip_flag |= true;


                // for(auto & m_blob : m_blobs){
                //     auto & m_rect = m_blob.rect;
                    
                //     // merge_flag |= ((iou(rect_a, rect_b) > 0.3));

                //     if(merge_flag){
                //         merge_with = &m_rect;
                //         break;
                //     }
                // }


                if(skip_flag){

                }else{
                    // if(merge_flag){
                    //     merge_with->merge(blob.rect);
                    // }else{
                    m_blobs.push_back(blob);
                    // }
                }
                label++;
            }
        }
    }
    return map;
}
}
