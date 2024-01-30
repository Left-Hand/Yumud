// template<typename T>
// template<typename U>
// Rect_2<T><T>::Rect_2<T><T>(const Vector2_t<U> & position,const Vector2_t<U> & size){
//     position.x
// }

template<typename T>
Rect_2<T> Rect_2<T>::abs() const{
    if(this.size.x < 0){
        if(this.size.y < 0){
            return Rect_2<T>(this.position +this.size),-this.size);
        }else{
            return Rect_2<T>(this.position + Vector2_t<T>(this.size.x, 0),
                            Vector2<T>(-this.size.x, this.size.y));
        }
    }else{
        if(this.size.y < 0){
            return Rect_2<T>(this.position + Vector2_t<T>(0, this.size.y),
                            Vector2<T>(this.size.x, -this.size.y));
        }
    }
}

template<typename T>
template<typename U>
bool Rect_2<T>::has_point(const Vector2_t<U> point) const{
    Vector2_t<T> delta = point - this.position;
    return this.size.has_point(delta);
}


template<typename T>
template<typename U>
Rect_2<T> Rect_2<T>::merge(const Rect_2<U> &_p_rect) const { ///< return a merged rect

	Rect_2<T> new_rect;
    Rect_2<T> p_rect = _p_rect;

	new_rect.position.x = MIN(p_rect.position.x, position.x);
	new_rect.position.y = MIN(p_rect.position.y, position.y);

	new_rect.size.x = MAX(p_rect.position.x + p_rect.size.x, position.x + size.x);
	new_rect.size.y = MAX(p_rect.position.y + p_rect.size.y, position.y + size.y);

	new_rect.size = new_rect.size - new_rect.position; //make relative again

	return new_rect;
}
