/*************************************************************************************************************
 * Matrix_t Class
 *  Contain the matrix class definition and operation.
 * 
 *  Notes:
 *    a. The matrix data is a 2 dimensional array, with structure:
 *      ->  0 <= i16row <= MATRIX_MAXIMUM_SIZE
 *      ->  0 <= i16col <= MATRIX_MAXIMUM_SIZE
 *      ->  floatData[MATRIX_MAXIMUM_SIZE][MATRIX_MAXIMUM_SIZE] is the memory
 *           representation of the matrix. We only use the first i16row-th
 *           and first i16col-th memory for the matrix data. The rest is unused.
 * 
 *    b. Indexing start from 0, there are 3 ways to access the data:
 *          1. A(idxRow,idxCol)       <-- The preferred way. With bounds checking.
 *          2. A[idxRow][idxCol]      <-- Slow, not recommended, but makes a cute code. With bounds checking.
 *          3. A._at(idxRow,idxCol)   <-- Just for internal function usage. Without bounds checking.
 *       
 *       Accessing index start from 0 until i16row/i16col, that is:
 *          (0 <= idxRow < i16row)     and     (0 <= idxRow < i16col).
 * 
 *    See below at "Data structure of Matrix_t class" at private member class definition for more information!
 * 
 * 
 * Class Matrix_t Versioning:
 *    v0.10 (2020-04-29), {PNb}:
 *      - Add NoInitMatZero for _outp matrix initialization in ForwardSubtitution.
 *      - Fixing bug in HouseholderTransformQR function where _outp & _vectTemp need to be initialized
 *          with zero.
 *      - Change comparison with zero in these functions using float_prec_ZERO_ECO:
 *          -> operator ==
 *          -> operator /
 *          -> vRoundingElementToZero
 *          -> RoundingMatrixToZero
 *      - No need to check ABS(_normM) as non-zero in bNormVector().
 *      - Add operator != (checking with float_prec_ZERO_ECO).
 *      - Add MatIdentity() function.
 * 
 *    v0.9 (2020-04-28), {PNb}:
 *      - Set all function as inline function (insert oprah meme here).
 *      - Rework matrix library to increase readability.
 *      - Make every parameters and function as reference as much as possible.
 *      - Make every member function as a const function as possible.
 *      - Add () operator overload to access the matrix data, so we can do both:
 *         -> A[0][3] = access 1st row, 4th column of matrix data.
 *         -> A(2,0)  = access 3rd row, 1st column of matrix data. < this is the way.
 *       - Remove (bug) a remnant of ancient code where we haven't implemented NoInitMatZero:
 *          T floatData[MATRIX_MAXIMUM_SIZE][MATRIX_MAXIMUM_SIZE] = {{0}};
 *         Into:
 *          T floatData[MATRIX_MAXIMUM_SIZE][MATRIX_MAXIMUM_SIZE];
 *         We can get very nice speed up (MPC benchmark @2020-04-27) from 698 us to 414 us!
 *       - Implement copy constructor               (more sweet speed up, 414 us -> 382 us!).
 *       - Implement assignment operator       (more more sweet speed up, 382 us -> 327 us!).
 *       - Remove Copy() function, use assignment operator instead.
 *       - Change MATRIX_USE_BOUND_CHECKING -> MATRIX_USE_BOUNDS_CHECKING (see ...BOUND*S*_CH...).
 *  
 *    v0.8 (2020-03-26), {PNb}:
 *      - Change indexing from int32_t to int16_t.
 *      - Add way to initialize matrix with existing T array.
 *      - Add enum InitZero.
 *      - Make temporary matrix initialization inside almost all method with 
 *          NoInitMatZero argument.
 *      - Remove the 1 index buffer reserve in bMatrixIsValid function.
 *      - Add bMatrixIsPositiveDefinite method to check the positive 
 *          (semi)definiteness of a matrix.
 *      - Add GetDiagonalEntries method.
 *      - Change SYSTEM_IMPLEMENTATION_EMBEDDED_NO_PRINT into
 *          SYSTEM_IMPLEMENTATION_EMBEDDED_CUSTOM, and make vPrint and
 *          vPrintFull as function declaration (the user must define that
 *          function somewhere). 
 * 
 *    v0.7 (2020-02-23), {PNb}:
 *      - Make the matrix class interface in English (at long last, yay?).
 * 
 * 
 *** Documentation below is for tracking purpose *************************************
 * 
 *    v0.6 (2020-01-16), {PNb}:
 *      - Tambahkan sanity check saat pengecekan MATRIX_PAKAI_BOUND_CHECKING 
 *          dengan membandingkan baris & kolom dengan MATRIX_MAXIMUM_SIZE.
 *      - Menambahkan pengecekan matrix untuk operasi dasar antar matrix (*,+,-).
 * 
 *    v0.5 (2020-01-14), {PNb}:
 *      - Buat file matrix.cpp (akhirnya!) untuk definisi fungsi di luar class.
 *      - Tambahkan operator overloading untuk operasi negatif matrix (mis. a = -b).
 *      - Tambahkan operator overloading untuk operasi penjumlahan & pengurangan 
 *          dengan scalar.
 *      - Ubah evaluasi MATRIX_PAKAI_BOUND_CHECKING menggunakan ASSERT.
 *      - Tambahkan pengecekan index selalu positif di MATRIX_PAKAI_BOUND_CHECKING.
 * 
 *    v0.4 (2020-01-10), {PNb}:
 *      - Tambahkan rounding to zero sebelum operasi sqrt(x) untuk menghindari
 *          kasus x = 0-
 *      - Fungsi QRDec mengembalikan Q' dan R (user perlu melakukan transpose
 *          lagi setelah memanggil QRDec untuk mendapatkan Q).
 *      - Menambahkan pengecekan hasil HouseholderTransformQR di dalam QRDec.
 *      - Tambah warning jika MATRIX_PAKAI_BOUND_CHECKING dinonaktifkan.
 * 
 *    v0.3_engl (2019-12-31), {PNb}:
 *      - Modifikasi dokumentasi kode buat orang asing.
 * 
 *    v0.3 (2019-12-25), {PNb}:
 *      - Menambahkan fungsi back subtitution untuk menyelesaikan permasalahan 
 *          persamaan linear Ax = B. Dengan A matrix segitiga atas & B vektor.
 *      - Memperbaiki bug pengecekan MATRIX_PAKAI_BOUND_CHECKING pada indexing kolom.
 *      - Menambahkan fungsi QR Decomposition (via Householder Transformation).
 *      - Menambahkan fungsi Householder Transformation.
 *      - Menghilangkan warning 'implicit conversion' untuk operasi pembandingan
 *          dengan float_prec_ZERO.
 *      - Menambahkan function overloading operasi InsertSubMatrix, untuk
 *          operasi insert dari SubMatrix ke SubMatrix.
 *      - Saat inisialisasi, matrix diisi nol (melalui vIsiHomogen(T(0))).
 *      - Menambahkan function overloading operator '/' dengan scalar.
 * 
 *    v0.2 (2019-11-30), {PNb}:
 *      - Fungsi yang disupport:
 *          - Operator ==
 *          - Normalisasi matrix
 *          - Cholesky Decomposition
 *          - InsertSubMatrix
 *          - InsertVector
 * 
 *    v0.1 (2019-11-29), {PNb}: 
 *      - Fungsi yang disupport:
 *          - Operasi matrix dasar
 *          - inverse
 *          - Cetak
 * 
 * See https://github.com/pronenewbits for more!
 ************************************************************************************************************/
#ifndef MATRIX_H
#define MATRIX_H

#include "kconfig.h"
#include "memory.h"

#include "sys/math/real.hpp"
#include "sys/stream/ostream.hpp"

template<arithmetic T>
class Matrix_t
{
public:

    /* --------------------------------------------- Basic Matrix_t Class functions --------------------------------------------- */
    /* Init empty matrix size _i16row x _i16col */
    constexpr Matrix_t(const int16_t _i16row, const int16_t _i16col);

    /* Init matrix size _i16row x _i16col with entries initData */
    constexpr Matrix_t(const int16_t _i16row, const int16_t _i16col, const T* initData);

    /* Copy constructor (for this operation --> A(B)) (copy B into A) */
    constexpr Matrix_t(const Matrix_t& old_obj);

    
    /* Assignment operator (for this operation --> A = B) (copy B into A) */
    constexpr Matrix_t& operator = (const Matrix_t & other);

    /* Get internal state */
    __fast_inline constexpr int16_t rows(void) const { return this->i16row; }
    __fast_inline constexpr int16_t cols(void) const { return this->i16col; }
    
    
    /* ------------------------------------------- Matrix_t entry accessing functions ------------------------------------------- */
    /* For example: A(1,2) access the 1st row and 2nd column data of matrix A <--- The preferred way to access the matrix */
    T & operator () (const int16_t _row, const int16_t _col);
    const T & operator () (const int16_t _row, const int16_t _col) const;
    
    /* For example: A[1][2] access the 1st row and 2nd column data of matrix A <-- The awesome way */
    class Proxy {
        public:
            T & operator [] (const int16_t _col);
            T operator [] (const int16_t _col) const;
        private:
            Proxy(T* _inpArr, const int16_t _maxCol) { _array.ptr = _inpArr; this->_maxCol = _maxCol; }
            Proxy(const T* _inpArr, const int16_t _maxCol) { _array.cptr = _inpArr; this->_maxCol = _maxCol; }
            union { /* teehee xp */
                const T* cptr;
                T* ptr;
            } _array;
            int16_t _maxCol;

            friend class Matrix_t;
    };
    Proxy operator [] (const int16_t _row);
    const Proxy operator [] (const int16_t _row) const;
    
    
    /* ----------------------------------------- Matrix_t checking function declaration ----------------------------------------- */
    operator bool(void) const;
    void invalid(void);
    /* --------------------------------------------- Matrix_t elementary operations --------------------------------------------- */
    bool operator == (const Matrix_t& _compare) const;
    bool operator != (const Matrix_t& _compare) const;
    Matrix_t operator - (void) const;
    Matrix_t operator + (const T _scalar) const;
    Matrix_t operator - (const T _scalar) const;
    Matrix_t operator * (const T _scalar) const;
    Matrix_t operator / (const T _scalar) const;
    Matrix_t operator + (const Matrix_t& _matAdd) const;
    Matrix_t operator - (const Matrix_t& _matSub) const;
    Matrix_t operator * (const Matrix_t& _matMul) const;
    /* Declared outside class below */
    /* inline Matrix_t operator + (const T _scalar, Matrix_t _mat); */
    /* inline Matrix_t operator - (const T _scalar, Matrix_t _mat); */
    /* inline Matrix_t operator * (const T _scalar, Matrix_t _mat); */
    /* ----------------------------------------------- Simple Matrix_t operations ----------------------------------------------- */
    void vRoundingElementToZero(const int16_t _i, const int16_t _j);
    Matrix_t RoundingMatrixToZero(void);
    void vSetHomogen(const T _val);
    void vSetToZero(void);
    void vSetRandom(const int32_t _maxRand, const int32_t _minRand);
    void vSetDiag(const T _val);
    void vSetIdentity(void);
    Matrix_t transpose(void);
    bool bNormVector(void);
    /* ------------------------------------------ Matrix_t/Vector insertion operations ------------------------------------------ */
    Matrix_t InsertVector(const Matrix_t& _Vector, const int16_t _posCol);
    Matrix_t InsertSubMatrix(const Matrix_t& _subMatrix, const int16_t _posRow, const int16_t _posCol);
    Matrix_t InsertSubMatrix(const Matrix_t& _subMatrix, const int16_t _posRow, const int16_t _posCol,
                           const int16_t _lenRow, const int16_t _lenColumn);
    Matrix_t InsertSubMatrix(const Matrix_t& _subMatrix, const int16_t _posRow, const int16_t _posCol,
                           const int16_t _posRowSub, const int16_t _posColSub,
                           const int16_t _lenRow, const int16_t _lenColumn);
    /* ---------------------------------------------------- Big operations ---------------------------------------------------- */
    /* Matrix_t invertion using Gauss-Jordan algorithm */
    Matrix_t inverse(void) const;
    /* Check the definiteness of a matrix */
    bool bMatrixIsPositiveDefinite(const bool checkPosSemidefinite = false) const;
    /* Return the vector (Mx1 matrix) correspond with the diagonal entries of 'this' */
    Matrix_t GetDiagonalEntries(void) const;
    /* Do the Cholesky Decomposition using Cholesky-Crout algorithm, return 'L' matrix */
    Matrix_t CholeskyDec(void) const;
    /* Do Householder Transformation for QR Decomposition operation */
    Matrix_t HouseholderTransformQR(const int16_t _rowTransform, const int16_t _colTransform);
    /* Do QR Decomposition for matrix using Householder Transformation */
    bool QRDec(Matrix_t& Qt, Matrix_t& R) const;
    /* Do back-subtitution for upper triangular matrix A & column matrix B:
     * x = BackSubtitution(&A, &B)          ; for Ax = B
     */
    Matrix_t BackSubtitution(const Matrix_t& A, const Matrix_t& B) const;
    /* Do forward-subtitution for lower triangular matrix A & column matrix B:
     * x = ForwardSubtitution(&A, &B)       ; for Ax = B
     */
    Matrix_t ForwardSubtitution(const Matrix_t& A, const Matrix_t& B) const;
    /* ----------------------------------------------- Matrix_t printing function ----------------------------------------------- */

    T & at(const int16_t _row, const int16_t _col) { return this->floatData[_row][_col]; }
    const T & at(const int16_t _row, const int16_t _col) const { return this->floatData[_row][_col]; }
    
private:
    /* Data structure of Matrix_t class:
     *  0 <= i16row <= MATRIX_MAXIMUM_SIZE      ; i16row is the row of the matrix. i16row is invalid if (i16row == -1)
     *  0 <= i16col <= MATRIX_MAXIMUM_SIZE      ; i16col is the column of the matrix. i16col is invalid if (i16col == -1)
     * 
     * Accessing index start from 0 until i16row/i16col, that is:
     *  (0 <= idxRow < i16row)     and     (0 <= idxCol < i16col).
     * There are 3 ways to access the data:
     *  1. A[idxRow][idxCol]          <-- Slow, not recommended, but make a cute code. With bounds checking.
     *  2. A(idxRow, idxCol)          <-- The preferred way. With bounds checking.
     *  3. A._at(idxRow, idxCol)      <-- Just for internal function usage. Without bounds checking.
     * 
     * floatData[MATRIX_MAXIMUM_SIZE][MATRIX_MAXIMUM_SIZE] is the memory representation of the matrix. We only use the
     *  first i16row-th and first i16col-th memory for the matrix data. The rest is unused.
     * 
     * This configuration might seems wasteful (yes it is). But with this, we can make the matrix library code as cleanly
     *  as possible (like I said in the github page, I've made decision to sacrifice speed & performance to get best code
     *  readability I could get).
     * 
     * You could change the data structure of floatData if you want to make the implementation more memory efficient.
     */
    int16_t i16row;
    int16_t i16col;
    T floatData[MATRIX_MAXIMUM_SIZE][MATRIX_MAXIMUM_SIZE];
    
    /* Private way to access floatData without bound checking.
     *  TODO: For Matrix_t member function we could do the bound checking once at the beginning of the function, and use this
     *          to access the floatData instead of (i,j) operator. From preliminary experiment doing this only on elementary
     *          operation (experiment @2020-04-27), we can get up to 45% computation boost!!! (MPC benchmark 414 us -> 226 us)!
     */
};

template<arithmetic T>
inline Matrix_t<T> operator + (const T _scalar, const Matrix_t<T>& _mat);

template<arithmetic T>
inline Matrix_t<T> operator - (const T _scalar, const Matrix_t<T>& _mat);

template<arithmetic T>
inline Matrix_t<T> operator * (const T _scalar, const Matrix_t<T>& _mat);

template<arithmetic T>
inline Matrix_t<T> MatIdentity(const int16_t _i16size);

template<arithmetic T>
constexpr Matrix_t<T>::Matrix_t(const int16_t _i16row, const int16_t _i16col) {
    this->i16row = _i16row;
    this->i16col = _i16col;
}


template<arithmetic T>
constexpr Matrix_t<T>::Matrix_t(const int16_t _i16row, const int16_t _i16col, const T* initData) {
    this->i16row = _i16row;
    this->i16col = _i16col;
    
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            (*this)(_i,_j) = *initData;
            initData++;
        }
    }
}

template<arithmetic T>
constexpr Matrix_t<T>::Matrix_t(const Matrix_t<T>& old_obj) {
    /* For copy contructor, we only need to copy (_i16row x _i16col) submatrix, there's no need to copy all data */
    this->i16row = old_obj.i16row;
    this->i16col = old_obj.i16col;
    
    const T *sourc = old_obj.floatData[0];
    T *desti = this->floatData[0];

    for (int16_t _i = 0; _i < i16row; _i++) {
        /* Still valid with invalid matrix ((i16row == -1) or (i16col == -1)) */
        memcpy(desti, sourc, sizeof(T)*size_t((this->i16col)));
        sourc += (MATRIX_MAXIMUM_SIZE);
        desti += (MATRIX_MAXIMUM_SIZE);
    }
}

template<arithmetic T>
constexpr Matrix_t<T>& Matrix_t<T>::operator = (const Matrix_t<T>& obj) {
    /* For assignment operator, we only need to copy (_i16row x _i16col) submatrix, there's no need to copy all data */
    this->i16row = obj.i16row;
    this->i16col = obj.i16col;
    
    const T *sourc = obj.floatData[0];
    T *desti = this->floatData[0];

    for (int16_t _i = 0; _i < i16row; _i++) {
        /* Still valid with invalid matrix ((i16row == -1) or (i16col == -1)) */
        memcpy(desti, sourc, sizeof(T)*size_t((this->i16col)));
        sourc += (MATRIX_MAXIMUM_SIZE);
        desti += (MATRIX_MAXIMUM_SIZE);
    }
    
    return *this;
}

/* ---------------------------------------- Matrix_t entry accessing functions ---------------------------------------- */
/* ---------------------------------------- Matrix_t entry accessing functions ---------------------------------------- */

/* The preferred method to access the matrix data (boring code) */

template<arithmetic T>
inline T& Matrix_t<T>::operator () (const int16_t _row, const int16_t _col) {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_row >= 0) && (_row < this->i16row) && (_row < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at row evaluation)");
        MATRIX_ASSERT((_col >= 0) && (_col < this->i16col) && (_col < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at column _column)");
    #endif
    return this->floatData[_row][_col];
}

template<arithmetic T>
inline const T & Matrix_t<T>::operator () (const int16_t _row, const int16_t _col) const {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_row >= 0) && (_row < this->i16row) && (_row < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at row evaluation)");
        MATRIX_ASSERT((_col >= 0) && (_col < this->i16col) && (_col < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at column _column)");
    #endif
    return this->floatData[_row][_col];
}

/* Ref: https://stackoverflow.com/questions/6969881/operator-overload
 * Modified to be lvalue modifiable (I know this is so dirty, but it makes the code so FABULOUS XD)
 */

template<arithmetic T>
inline T & Matrix_t<T>::Proxy::operator [] (const int16_t _col) {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_col >= 0) && (_col < this->_maxCol) && (_col < MATRIX_MAXIMUM_SIZE),
                "Matrix_t index out-of-bounds (at column evaluation)");
    #endif
    return _array.ptr[_col];
}

template<arithmetic T>
inline T Matrix_t<T>::Proxy::operator [] (const int16_t _col) const {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_col >= 0) && (_col < this->_maxCol) && (_col < MATRIX_MAXIMUM_SIZE),
                "Matrix_t index out-of-bounds (at column evaluation)");
    #endif
    return _array.cptr[_col];
}

template<arithmetic T>
inline Matrix_t<T>::Proxy Matrix_t<T>::operator [] (const int16_t _row) {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_row >= 0) && (_row < this->i16row) && (_row < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at row evaluation)");
    #endif
    return Proxy(floatData[_row], this->i16col);  /* Parsing column index for bound checking */
}

template<arithmetic T>

inline const Matrix_t<T>::Proxy Matrix_t<T>::operator [] (const int16_t _row) const {
    #if (defined(MATRIX_USE_BOUNDS_CHECKING))
        MATRIX_ASSERT((_row >= 0) && (_row < this->i16row) && (_row < MATRIX_MAXIMUM_SIZE),
               "Matrix_t index out-of-bounds (at row evaluation)");
    #endif
    return Proxy(floatData[_row], this->i16col);  /* Parsing column index for bound checking */
}


/* -------------------------------------- Matrix_t checking function declaration -------------------------------------- */
/* -------------------------------------- Matrix_t checking function declaration -------------------------------------- */

template<arithmetic T>
inline Matrix_t<T>::operator bool(void) const{
    /* Check whether the matrix is valid or not */
    if ((this->i16row > 0) && (this->i16row <= MATRIX_MAXIMUM_SIZE) &&
        (this->i16col > 0) && (this->i16col <= MATRIX_MAXIMUM_SIZE))
    {
        return true;
    } else {
        return false;
    }
}

template<arithmetic T>
inline void Matrix_t<T>::invalid(void) {
    this->i16row = -1;
    this->i16col = -1;
}

/* ------------------------------------------ Matrix_t elementary operations ------------------------------------------ */
/* ------------------------------------------ Matrix_t elementary operations ------------------------------------------ */
/* TODO: We could do loop unrolling here for elementary, simple, and matrix insertion operations. It *might* speed up
 *        the computation time up to 20-30% for processor with FMAC and cached CPU (I still mull on this because
 *        the code will be awful).
 */

template<arithmetic T>
inline bool Matrix_t<T>::operator == (const Matrix_t<T> & _compare) const {
    if ((this->i16row != _compare.i16row) || (this->i16col != _compare.i16col)) {
        return false;
    }

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            if (ABS((*this)(_i,_j) - _compare(_i,_j)) > T(float_prec_ZERO_ECO)) {
                return false;
            }
        }
    }
    return true;
}

template<arithmetic T>
inline bool Matrix_t<T>::operator != (const Matrix_t<T> & _compare) const {
    return (!(*this == _compare));
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator - (void) const {
    Matrix_t<T> _outp(this->i16row, this->i16col, Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = -(*this)(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator + (const T _scalar) const {
    Matrix_t _outp(this->i16row, this->i16col, Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) + _scalar;
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator - (const T _scalar) const {
    Matrix_t<T> _outp(this->i16row, this->i16col, Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) - _scalar;
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator * (const T _scalar) const {
    Matrix_t<T> _outp(this->i16row, this->i16col, Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) * _scalar;
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator / (const T _scalar) const {
    Matrix_t<T> _outp(this->i16row, this->i16col, Matrix_t<T>::NoInitMatZero);

    if (ABS(_scalar) < T(float_prec_ZERO_ECO)) {
        _outp.invalid();
        return _outp;
    }
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) / _scalar;
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> operator + (const T _scalar, const Matrix_t<T>& _mat) {
    Matrix_t<T> _outp(_mat.rows()(), _mat.cols()(), Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < _mat.rows()(); _i++) {
        for (int16_t _j = 0; _j < _mat.cols()(); _j++) {
            _outp(_i,_j) = _scalar + _mat(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> operator - (const T _scalar, const Matrix_t<T>& _mat) {
    Matrix_t<T> _outp(_mat.rows()(), _mat.cols()(), Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < _mat.rows()(); _i++) {
        for (int16_t _j = 0; _j < _mat.cols()(); _j++) {
            _outp(_i,_j) = _scalar - _mat(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> operator * (const T _scalar, const Matrix_t<T>& _mat) {
    Matrix_t<T> _outp(_mat.rows()(), _mat.cols()(), Matrix_t<T>::NoInitMatZero);

    for (int16_t _i = 0; _i < _mat.rows()(); _i++) {
        for (int16_t _j = 0; _j < _mat.cols()(); _j++) {
            _outp(_i,_j) = _scalar * _mat(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator + (const Matrix_t<T>& _matAdd) const {
    Matrix_t<T> _outp(this->i16row, this->i16col);
    if ((this->i16row != _matAdd.i16row) || (this->i16col != _matAdd.i16col)) {
        _outp.invalid();
        return _outp;
    }

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) + _matAdd(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator - (const Matrix_t<T>& _matSub) const {
    Matrix_t<T> _outp(this->i16row, this->i16col);
    if ((this->i16row != _matSub.i16row) || (this->i16col != _matSub.i16col)) {
        _outp.invalid();
        return _outp;
    }

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_i,_j) = (*this)(_i,_j) - _matSub(_i,_j);
        }
    }
    return _outp;
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::operator * (const Matrix_t<T>& _matMul) const {
    Matrix_t<T> _outp(this->i16row, _matMul.i16col);
    if ((this->i16col != _matMul.i16row)) {
        _outp.invalid();
        return _outp;
    }

    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < _matMul.i16col; _j++) {
            _outp(_i,_j) = T(0);
            for (int16_t _k = 0; _k < this->i16col; _k++) {
                _outp(_i,_j) += ((*this)(_i,_k) * _matMul(_k,_j));
            }
        }
    }
    return _outp;
}


/* -------------------------------------------- Simple Matrix_t operations -------------------------------------------- */
/* -------------------------------------------- Simple Matrix_t operations -------------------------------------------- */

template<arithmetic T>
inline void Matrix_t<T>::vRoundingElementToZero(const int16_t _i, const int16_t _j) {
    if (ABS((*this)(_i,_j)) < T(float_prec_ZERO_ECO)) {
        (*this)(_i,_j) = T(0);
    }
}

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::RoundingMatrixToZero(void) {
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            if (ABS((*this)(_i,_j)) < T(float_prec_ZERO_ECO)) {
                (*this)(_i,_j) = T(0);
            }
        }
    }
    return (*this);
}

template<arithmetic T>
inline void Matrix_t<T>::vSetHomogen(const T _val) {
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            (*this)(_i,_j) = _val;
        }
    }
}

template<typename T>
inline void Matrix_t<T>::vSetToZero(void) {
    this->vSetHomogen(T(0));
}

template<arithmetic T>
inline void Matrix_t<T>::vSetRandom(const int32_t _maxRand, const int32_t _minRand) {
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            (*this)(_i,_j) = T((rand() % (_maxRand - _minRand + 1)) + _minRand);
        }
    }
}

template<arithmetic T>
inline void Matrix_t<T>::vSetDiag(const T _val) {
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            if (_i == _j) {
                (*this)(_i,_j) = _val;
            } else {
                (*this)(_i,_j) = T(0);
            }
        }
    }
}

template<arithmetic T>
inline void Matrix_t<T>::vSetIdentity(void) {
    this->vSetDiag(T(1));
}

template<arithmetic T>
inline Matrix_t<T> MatIdentity(const int16_t _i16size) {
    Matrix_t _outp(_i16size, _i16size, Matrix_t<T>::NoInitMatZero);
    _outp.vSetDiag(T(1));   
    return _outp;
}


/* Return the transpose of the matrix */
template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::transpose(void) {
    Matrix_t<T> _outp(this->i16col, this->i16row);
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _outp(_j,_i) = (*this)(_i,_j);
        }
    }
    return _outp;
}

/* Normalize the vector */
template<arithmetic T>
inline bool Matrix_t<T>::bNormVector(void) {
    T _normM = T(0);
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            _normM = _normM + ((*this)(_i,_j) * (*this)(_i,_j));
        }
    }

    /* Rounding to zero to avoid case where sqrt(0-), and _normM always positive */
    if (_normM < T(float_prec_ZERO)) {
        return false;
    }
    _normM = sqrt(_normM);
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        for (int16_t _j = 0; _j < this->i16col; _j++) {
            (*this)(_i,_j) /= _normM;
        }
    }
    return true;
}


/* --------------------------------------- Matrix_t/Vector insertion operations --------------------------------------- */
/* --------------------------------------- Matrix_t/Vector insertion operations --------------------------------------- */

/* Insert vector into matrix at _posCol position
 * Example: A = Matrix_t 3x3, B = Vector 3x1
 *
 *  C = A.InsertVector(B, 1);
 *
 *  A = [A00  A01  A02]     B = [B00]
 *      [A10  A11  A12]         [B10]
 *      [A20  A21  A22]         [B20]
 *
 *  C = [A00  B00  A02]
 *      [A10  B10  A12]
 *      [A20  B20  A22]
 */
template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::InsertVector(const Matrix_t& _Vector, const int16_t _posCol) {
    Matrix_t<T> _outp(*this);
    if ((_Vector.i16row > this->i16row) || (_Vector.i16col+_posCol > this->i16col)) {
        /* Return false */
        _outp.invalid();
        return _outp;
    }
    for (int16_t _i = 0; _i < _Vector.i16row; _i++) {
        _outp(_i,_posCol) = _Vector(_i,0);
    }
    return _outp;
}

/* Insert submatrix into matrix at _posRow & _posCol position
 * Example: A = Matrix_t 4x4, B = Matrix_t 2x3
 *
 *  C = A.InsertSubMatrix(B, 1, 1);
 *
 *  A = [A00  A01  A02  A03]    B = [B00  B01  B02]
 *      [A10  A11  A12  A13]        [B10  B11  B12]
 *      [A20  A21  A22  A23]
 *      [A30  A31  A32  A33]
 *
 *
 *  C = [A00  A01  A02  A03]
 *      [A10  B00  B01  B02]
 *      [A20  B10  B11  B12]
 *      [A30  A31  A32  A33]
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::InsertSubMatrix(const Matrix_t& _subMatrix, const int16_t _posRow, const int16_t _posCol) {
    Matrix_t<T> _outp(*this);
    if (((_subMatrix.i16row+_posRow) > this->i16row) || ((_subMatrix.i16col+_posCol) > this->i16col)) {
        /* Return false */
        _outp.invalid();
        return _outp;
    }
    for (int16_t _i = 0; _i < _subMatrix.i16row; _i++) {
        for (int16_t _j = 0; _j < _subMatrix.i16col; _j++) {
            _outp(_i + _posRow,_j + _posCol) = _subMatrix(_i,_j);
        }
    }
    return _outp;
}

/* Insert the first _lenRow-th and first _lenColumn-th submatrix into matrix;
 *  at the matrix's _posRow and _posCol position.
 * 
 * Example: A = Matrix_t 4x4, B = Matrix_t 2x3
 *
 *  C = A.InsertSubMatrix(B, 1, 1, 2, 2);
 *
 *  A = [A00  A01  A02  A03]    B = [B00  B01  B02]
 *      [A10  A11  A12  A13]        [B10  B11  B12]
 *      [A20  A21  A22  A23]
 *      [A30  A31  A32  A33]
 *
 *
 *  C = [A00  A01  A02  A03]
 *      [A10  B00  B01  A13]
 *      [A20  B10  B11  A23]
 *      [A30  A31  A32  A33]
 */
template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::InsertSubMatrix(const Matrix_t<T>& _subMatrix, const int16_t _posRow, const int16_t _posCol,
                                      const int16_t _lenRow, const int16_t _lenColumn)
{
    Matrix_t _outp(*this);
    if (((_lenRow+_posRow) > this->i16row) || ((_lenColumn+_posCol) > this->i16col) ||
        (_lenRow > _subMatrix.i16row) || (_lenColumn > _subMatrix.i16col))
    {
        /* Return false */
        _outp.invalid();
        return _outp;
    }
    for (int16_t _i = 0; _i < _lenRow; _i++) {
        for (int16_t _j = 0; _j < _lenColumn; _j++) {
            _outp(_i + _posRow,_j + _posCol) = _subMatrix(_i,_j);
        }
    }
    return _outp;
}

/* Insert the _lenRow & _lenColumn submatrix, start from _posRowSub & _posColSub submatrix;
 *  into matrix at the matrix's _posRow and _posCol position.
 * 
 * Example: A = Matrix_t 4x4, B = Matrix_t 2x3
 *
 *  C = A.InsertSubMatrix(B, 1, 1, 0, 1, 1, 2);
 *
 *  A = [A00  A01  A02  A03]    B = [B00  B01  B02]
 *      [A10  A11  A12  A13]        [B10  B11  B12]
 *      [A20  A21  A22  A23]
 *      [A30  A31  A32  A33]
 *
 *  C = [A00  A01  A02  A03]
 *      [A10  B01  B02  A13]
 *      [A20  A21  A22  A23]
 *      [A30  A31  A32  A33]
 */
template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::InsertSubMatrix(const Matrix_t<T>& _subMatrix, const int16_t _posRow, const int16_t _posCol,
                        const int16_t _posRowSub, const int16_t _posColSub,
                        const int16_t _lenRow, const int16_t _lenColumn)
{
    Matrix_t _outp(*this);
    if (((_lenRow+_posRow) > this->i16row) || ((_lenColumn+_posCol) > this->i16col) ||
        ((_posRowSub+_lenRow) > _subMatrix.i16row) || ((_posColSub+_lenColumn) > _subMatrix.i16col))
    {
        /* Return false */
        _outp.invalid();
        return _outp;
    }
    for (int16_t _i = 0; _i < _lenRow; _i++) {
        for (int16_t _j = 0; _j < _lenColumn; _j++) {
            _outp(_i + _posRow,_j + _posCol) = _subMatrix(_posRowSub+_i,_posColSub+_j);
        }
    }
    return _outp;
}


/* ------------------------------------------------- Big operations ------------------------------------------------- */
/* ------------------------------------------------- Big operations ------------------------------------------------- */

/* inverse operation using Gauss-Jordan algorithm */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::inverse(void) const {
    Matrix_t _outp(this->i16row, this->i16col);
    Matrix_t _temp(*this);
    _outp.vSetIdentity();
    
    /* Gauss Elimination... */
    for (int16_t _j = 0; _j < (_temp.i16row)-1; _j++) {
        for (int16_t _i = _j+1; _i < _temp.i16row; _i++) {
            if (ABS(_temp(_j,_j)) < T(float_prec_ZERO)) {
                /* Matrix_t is non-invertible */
                _outp.invalid();
                return _outp;
            }

            T _tempfloat = _temp(_i,_j) / _temp(_j,_j);

            for (int16_t _k = 0; _k < _temp.i16col; _k++) {
                _temp(_i,_k) -= (_temp(_j,_k) * _tempfloat);
                _outp(_i,_k) -= (_outp(_j,_k) * _tempfloat);

                _temp.vRoundingElementToZero(_i, _k);
                _outp.vRoundingElementToZero(_i, _k);
            }

        }
    }

    #if (1)
        /* Here, the _temp matrix should be an upper triangular matrix.
         * But because of rounding error, it might not.
         */
        for (int16_t _i = 1; _i < _temp.i16row; _i++) {
            for (int16_t _j = 0; _j < _i; _j++) {
                _temp(_i,_j) = T(0);
            }
        }
    #endif


    /* Jordan... */
    for (int16_t _j = (_temp.i16row)-1; _j > 0; _j--) {
        for (int16_t _i = _j-1; _i >= 0; _i--) {
            if (ABS(_temp(_j,_j)) < T(float_prec_ZERO)) {
                /* Matrix_t is non-invertible */
                _outp.invalid();
                return _outp;
            }

            T _tempfloat = _temp(_i,_j) / _temp(_j,_j);
            _temp(_i,_j) -= (_temp(_j,_j) * _tempfloat);
            _temp.vRoundingElementToZero(_i, _j);

            for (int16_t _k = (_temp.i16row - 1); _k >= 0; _k--) {
                _outp(_i,_k) -= (_outp(_j,_k) * _tempfloat);
                _outp.vRoundingElementToZero(_i, _k);
            }
        }
    }


    /* Normalization */
    for (int16_t _i = 0; _i < _temp.i16row; _i++) {
        if (ABS(_temp(_i,_i)) < T(float_prec_ZERO)) {
            /* Matrix_t is non-invertible */
            _outp.invalid();
            return _outp;
        }

        T _tempfloat = _temp(_i,_i);
        _temp(_i,_i) = T(1);

        for (int16_t _j = 0; _j < _temp.i16row; _j++) {
            _outp(_i,_j) /= _tempfloat;
        }
    }
    return _outp;
}

/* Use elemetary row operation to reduce the matrix into upper triangular form
 *  (like in the first phase of gauss-jordan algorithm).
 * 
 * Useful if we want to check whether the matrix is positive definite or not
 *  (useful before calling CholeskyDec function).
 */

template<arithmetic T>
inline bool Matrix_t<T>::bMatrixIsPositiveDefinite(const bool checkPosSemidefinite) const {
    bool _posDef, _posSemiDef;
    Matrix_t _temp(*this);
    
    /* Gauss Elimination... */
    for (int16_t _j = 0; _j < (_temp.i16row)-1; _j++) {
        for (int16_t _i = _j+1; _i < _temp.i16row; _i++) {
            if (ABS(_temp(_j,_j)) < T(float_prec_ZERO)) {
                /* Q: Do we still need to check this?
                 * A: idk, it's 3 AM. I need sleep :<
                 * 
                 * NOTE TO FUTURE SELF: Confirm it!
                 */
                return false;
            }
            
            T _tempfloat = _temp(_i,_j) / _temp(_j,_j);
            
            for (int16_t _k = 0; _k < _temp.i16col; _k++) {
                _temp(_i,_k) -= (_temp(_j,_k) * _tempfloat);
                _temp.vRoundingElementToZero(_i, _k);
            }

        }
    }
    
    _posDef = true;
    _posSemiDef = true;
    for (int16_t _i = 0; _i < _temp.i16row; _i++) {
        if (_temp(_i,_i) < T(float_prec_ZERO)) {
            /* false if less than 0+ (zero included) */
            _posDef = false;
        }
        if (_temp(_i,_i) < -T(float_prec_ZERO)) {
            /* false if less than 0- (zero is not included) */
            _posSemiDef = false;
        }
    }
    
    if (checkPosSemidefinite) {
        return _posSemiDef;
    } else {
        return _posDef;
    }
}

/* For square matrix 'this' with size MxM, return vector Mx1 with entries
 * correspond with diagonal entries of 'this'.
 * 
 *   Example:    this = [a11 a12 a13]
 *                      [a21 a22 a23]
 *                      [a31 a32 a33]
 * 
 * out = this.GetDiagonalEntries() = [a11]
 *                                   [a22]
 *                                   [a33]
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::GetDiagonalEntries(void) const {
    Matrix_t _temp(this->i16row, 1);
    
    if (this->i16row != this->i16col) {
        _temp.invalid();
        return _temp;
    }
    for (int16_t _i = 0; _i < this->i16row; _i++) {
        _temp(_i,0) = (*this)(_i,_i);
    }
    return _temp;
}

/* Do the Cholesky Decomposition using Cholesky-Crout algorithm.
 * 
 *      A = L*L'     ; A = real, positive definite, and symmetry MxM matrix
 *
 *      L = A.CholeskyDec();
 *
 *      CATATAN! NOTE! The symmetry property is not checked at the beginning to lower
 *          the computation cost. The processing is being done on the lower triangular
 *          component of _A. Then it is assumed the upper triangular is inherently
 *          equal to the lower end.
 *          (as a side note, Scilab & MATLAB is using Lapack routines DPOTRF that process
 *           the upper triangular of _A. The result should be equal mathematically if A
 *           is symmetry).
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::CholeskyDec(void) const {
    T _tempFloat;

    /* Note that _outp need to be initialized as zero matrix */
    Matrix_t _outp(this->i16row, this->i16col);
    _outp.vSetToZero();
    if (this->i16row != this->i16col) {
        _outp.invalid();
        return _outp;
    }
    for (int16_t _j = 0; _j < this->i16col; _j++) {
        for (int16_t _i = _j; _i < this->i16row; _i++) {
            _tempFloat = (*this)(_i,_j);
            if (_i == _j) {
                for (int16_t _k = 0; _k < _j; _k++) {
                    _tempFloat = _tempFloat - (_outp(_i,_k) * _outp(_i,_k));
                }
                if (_tempFloat < -T(float_prec_ZERO)) {
                    /* Matrix_t is not positif (semi)definit */
                    _outp.invalid();
                    return _outp;
                }
                /* Rounding to zero to avoid case where sqrt(0-) */
                if (ABS(_tempFloat) < T(float_prec_ZERO)) {
                    _tempFloat = T(0);
                }
                _outp(_i,_i) = sqrt(_tempFloat);
            } else {
                for (int16_t _k = 0; _k < _j; _k++) {
                    _tempFloat = _tempFloat - (_outp(_i,_k) * _outp(_j,_k));
                }
                if (ABS(_outp(_j,_j)) < T(float_prec_ZERO)) {
                    /* Matrix_t is not positif definit */
                    _outp.invalid();
                    return _outp;
                }
                _outp(_i,_j) = _tempFloat / _outp(_j,_j);
            }
        }
    }
    return _outp;
}

/* Do the Householder Transformation for QR Decomposition operation.
 *              out = HouseholderTransformQR(A, i, j)
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::HouseholderTransformQR(const int16_t _rowTransform, const int16_t _colTransform) {
    T _tempFloat;
    T _xLen;
    T _x1;
    T _u1;
    T _vLen2;
    
    /* Note that _outp & _vectTemp need to be initialized as zero matrix */
    Matrix_t _outp(this->i16row, this->i16row);
    _outp.vSetToZero();
    Matrix_t _vectTemp(this->i16row, 1);
    _vectTemp.vSetToZero();
    
    if ((_rowTransform >= this->i16row) || (_colTransform >= this->i16col)) {
        _outp.invalid();
        return _outp;
    }
    
    /* Until here:
     *
     * _xLen    = ||x||            = sqrt(x1^2 + x2^2 + .. + xn^2)
     * _vLen2   = ||u||^2 - [u1^2] = x2^2 + .. + xn^2
     * _vectTemp= [0 0 0 .. x1=0 x2 x3 .. xn]'
     */
    _x1 = (*this)(_rowTransform,_colTransform);
    _xLen = _x1*_x1;
    _vLen2 = T(0);
    for (int16_t _i = _rowTransform+1; _i < this->i16row; _i++) {
        _vectTemp(_i,0) = (*this)(_i,_colTransform);

        _tempFloat = _vectTemp(_i,0) * _vectTemp(_i,0);
        _xLen  += _tempFloat;
        _vLen2 += _tempFloat;
    }
    _xLen = sqrt(_xLen);
    
    /* u1    = x1+(-sign(x1))*xLen */
    if (_x1 < T(0)) {
        _u1 = _x1+_xLen;
    } else {
        _u1 = _x1-_xLen;
    }
    
    /* Solve vlen2 & tempHH */
    _vLen2 += (_u1*_u1);
    _vectTemp(_rowTransform,0) = _u1;
    
    if (ABS(_vLen2) < T(float_prec_ZERO_ECO)) {
        /* x vector is collinear with basis vector e, return result = I */
        _outp.vSetIdentity();
    } else {
        /* P = -2*(u1*u1')/v_len2 + I */
        /* PR TODO: We need to investigate more on this */
        for (int16_t _i = 0; _i < this->i16row; _i++) {
            _tempFloat = _vectTemp(_i,0);
            if (ABS(_tempFloat) > T(float_prec_ZERO)) {
                for (int16_t _j = 0; _j < this->i16row; _j++) {
                    if (ABS(_vectTemp(_j,0)) > T(float_prec_ZERO)) {
                        _outp(_i,_j) = _vectTemp(_j,0);
                        _outp(_i,_j) = _outp(_i,_j) * _tempFloat;
                        _outp(_i,_j) = _outp(_i,_j) * (T(-2)/_vLen2);
                    }
                }
            }
            _outp(_i,_i) = _outp(_i,_i) + T(1);
        }
    }
    return _outp;
}

/* Do the QR Decomposition for matrix using Householder Transformation.
 *                      A = Q*R
 * 
 * PERHATIAN! CAUTION! The matrix calculated by this function are Q' and R (Q transpose and R).
 *  Because QR Decomposition usually used to calculate solution for least-squares equation
 *  (that need Q'), we don't do the transpose of Q inside this routine to lower the
 *  computation cost (user need to transpose outside if they want Q).
 * 
 * Example of using QRDec to solve least-squares:
 *                      Ax = b
 *                   (QR)x = b
 *                      Rx = Q'b    --> Afterward use back-subtitution to solve x
 */

template<arithmetic T>
inline bool Matrix_t<T>::QRDec(Matrix_t<T>& Qt, Matrix_t<T>& R) const {
    Matrix_t Qn(Qt.i16row, Qt.i16col);
    if ((this->i16row < this->i16col) || (Qt.i16roow != Qt.i16col) || (Qt.i16row != this->i16row) ||
        (R.i16row != this->i16row) || (R.i16col != this->i16col))
    {
        Qt.invalid();
        R.invalid();
        return false;
    }
    R = (*this);
    Qt.vSetIdentity();
    for (int16_t _i = 0; (_i < (this->i16row - 1)) && (_i < this->i16col-1); _i++) {
        Qn  = R.HouseholderTransformQR(_i, _i);
        if (!Qn.bMatrixIsValid()) {
            Qt.invalid();
            R.invalid();
            return false;
        }
        Qt = Qn * Qt;
        R  = Qn * R;
    }
#if (0)
    Qt.RoundingMatrixToZero();
    R.RoundingMatrixToZero();
#else
    Qt.RoundingMatrixToZero();
    for (int16_t _i = 1; ((_i < R.i16row) && (_i < R.i16col)); _i++) {
        for (int16_t _j = 0; _j < _i; _j++) {
            R(_i, _j) = T(0);
        }
    }
#endif
    
    /* Q = Qt.transpose */
    return true;
}

/* Do the back-subtitution operation for upper triangular matrix A & column matrix B to solve x:
 *                      Ax = B
 * 
 * x = BackSubtitution(&A, &B);
 *
 * CATATAN! NOTE! To lower the computation cost, we don't check that A is a upper triangular
 *  matrix (it's assumed that user already make sure of that before calling this routine).
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::BackSubtitution(const Matrix_t<T>& A, const Matrix_t<T>& B) const {
    Matrix_t _outp(A.i16row, 1);
    if ((A.i16row != A.i16col) || (A.i16row != B.i16row)) {
        _outp.invalid();
        return _outp;
    }

    for (int16_t _i = A.i16col-1; _i >= 0; _i--) {
        _outp(_i,0) = B(_i,0);
        for (int16_t _j = _i + 1; _j < A.i16col; _j++) {
            _outp(_i,0) = _outp(_i,0) - A(_i,_j)*_outp(_j,0);
        }
        if (ABS(A(_i,_i)) < T(float_prec_ZERO)) {
            _outp.invalid();
            return _outp;
        }
        _outp(_i,0) = _outp(_i,0) / A(_i,_i);
    }

    return _outp;
}

/* Do the forward-subtitution operation for lower triangular matrix A & column matrix B to solve x:
 *                      Ax = B
 * 
 * x = ForwardSubtitution(&A, &B);
 *
 * CATATAN! NOTE! To lower the computation cost, we don't check that A is a lower triangular
 *  matrix (it's assumed that user already make sure of that before calling this routine).
 */

template<arithmetic T>
inline Matrix_t<T> Matrix_t<T>::ForwardSubtitution(const Matrix_t<T> & A, const Matrix_t<T> & B) const {
    Matrix_t _outp(A.i16row, 1);
    if ((A.i16row != A.i16col) || (A.i16row != B.i16row)) {
        _outp.invalid();
        return _outp;
    }

    for (int16_t _i = 0; _i < A.i16row; _i++) {
        _outp(_i,0) = B(_i,0);
        for (int16_t _j = 0; _j < _i; _j++) {
            _outp(_i,0) = _outp(_i,0) - A(_i,_j)*_outp(_j,0);
        }
        if (ABS(A(_i,_i)) < T(float_prec_ZERO)) {
            _outp.invalid();
            return _outp;
        }
        _outp(_i,0) = _outp(_i,0) / A(_i,_i);
    }
    return _outp;
}

template<arithmetic T>
__inline OutputStream & operator<<(OutputStream & os, const Matrix_t<T> & mat){
    os << "[ ";
	for (int16_t _i = 0; _i < mat.rows(); _i++) {
        if(_i != 0) os << "  ";
		os << "[ ";
		for (int16_t _j = 0; _j < mat.cols(); _j++) {
			os << mat(_i,_j);
            if(_j == mat.cols() - 1) break;
            os << ", ";
		}
		os << " ]";
        if(_i == mat.rows() - 1) break;
        os << ",\r\n";
	}
    os << " ]";
    return os;
}

using Matrix = Matrix_t<real_t>;
using MatrixF = Matrix_t<float>;
using MatrixD = Matrix_t<float>;
    
#endif // MATRIX_H