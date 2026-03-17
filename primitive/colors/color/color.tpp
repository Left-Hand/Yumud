/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2024  Rstr1aN / Yumud                                    */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/*                                                                        */
/* Note: This file has been modified by Rstr1aN / Yumud.                  */
/**************************************************************************/


namespace ymd{

#define COLOR_ADD_SUB_MUL_OPERATOR(op) \
template <typename T> \
__fast_inline constexpr RGBA<T> operator op(const RGBA<T> &p_Color, const auto &rvalue){ \
    RGBA<T> final = p_Color; \
    final op##= rvalue; \
    return final; \
}\
\
template <typename T> \
__fast_inline constexpr RGBA<T> operator op(const auto &lvalue, const RGBA<T> &p_Color){ \
    RGBA<T> final = p_Color; \
    final op##= lvalue; \
    return final; \
}\
\
template <typename T> \
__fast_inline constexpr RGBA<T> operator op(const RGBA<T> &p_Color, const RGBA<T> &d_Color){ \
    RGBA<T> final = p_Color; \
    final op##= d_Color; \
    return final; \
}

COLOR_ADD_SUB_MUL_OPERATOR(+) 
COLOR_ADD_SUB_MUL_OPERATOR(-) 
COLOR_ADD_SUB_MUL_OPERATOR(*) 

#undef COLOR_ADD_SUB_MUL_OPERATOR




}