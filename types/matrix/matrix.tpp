


template <typename T>
Matrix::Matrix(std::initializer_list<T> row) {
    rows = 1;
    cols = row.size();
    data.push_back(std::vector<real_t>(row.begin(), row.end()));
}

template <typename T>
Matrix::Matrix(std::initializer_list<std::initializer_list<T>> matrix) {
    rows = matrix.size();
    cols = 0;
    for (const auto& row : matrix) {
        cols = std::max(cols, row.size());
    }
    for (const auto& row : matrix) {
        data.push_back(std::vector<real_t>(row.begin(), row.end()));
        data.back().resize(cols, static_cast<real_t>(0));
    }
}