#pragma once

#include "primitive/arithmetic/angular.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "core/utils/zero.hpp"


namespace ymd::sync{

template<typename T>
struct CellBase;

template<typename T>
struct Probe;


// 浮点数特化
template<std::floating_point T>
struct [[nodiscard]] CellBase<T> {
    CellBase(const T initial_value){
        set(initial_value);
    }

    CellBase(){
        set(0);
    }
    static_assert(sizeof(T) <= sizeof(size_t), "atomic is not lockfree");

    void set(const T& val) {
        value_.store(val, std::memory_order_release);
    }
    
    T get() const {
        return value_.load(std::memory_order_acquire);
    }
    
    // 原子比较交换操作
    bool compare_exchange(T& expected, T desired) {
        return value_.compare_exchange_weak(expected, desired, 
            std::memory_order_acq_rel);
    }
    
private:
    std::atomic<T> value_{static_cast<T>(0)};
};

// 整数特化
template<std::integral T>
struct [[nodiscard]] CellBase<T> {
    CellBase(const T initial_value){
        set(initial_value);
    }
    CellBase(){
        set(0);
    }
    void set(const T& val) {
        value_.store(val, std::memory_order_release);
    }
    
    T get() const {
        return value_.load(std::memory_order_acquire);
    }
    
    // 原子加法
    T fetch_add(T arg) {
        return value_.fetch_add(arg, std::memory_order_acq_rel);
    }
    
    // 原子减法
    T fetch_sub(T arg) {
        return value_.fetch_sub(arg, std::memory_order_acq_rel);
    }
    
    // 原子位操作
    T fetch_and(T arg) {
        return value_.fetch_and(arg, std::memory_order_acq_rel);
    }
    
    T fetch_or(T arg) {
        return value_.fetch_or(arg, std::memory_order_acq_rel);
    }
    
    // 原子比较交换
    bool compare_exchange(T& expected, T desired) {
        return value_.compare_exchange_weak(expected, desired,
        std::memory_order_acq_rel);
    }
    
private:
    std::atomic<T> value_{static_cast<T>(0)};
};

// 定点数特化
template<size_t Q, typename D>
struct [[nodiscard]] CellBase<fixed_t<Q, D>> {
    using T = fixed_t<Q, D>;
    static_assert(std::is_integral_v<D>, "Underlying type must be integral");

    CellBase(const T initial_value){
        set(initial_value);
    }
    CellBase(){
        set(0);
    }

    void set(const fixed_t<Q, D>& val) {
        bits_.store(val.to_bits(), std::memory_order_release);
    }
    
    fixed_t<Q, D> get() const {
        return fixed_t<Q, D>::from_bits(bits_.load(std::memory_order_acquire));
    }
    
    // 原子加法
    fixed_t<Q, D> fetch_add(const fixed_t<Q, D> arg) {
        D result_bits = bits_.fetch_add(arg.to_bits(), std::memory_order_acq_rel);
        return fixed_t<Q, D>(result_bits);
    }
    
    // 原子减法
    fixed_t<Q, D> fetch_sub(const fixed_t<Q, D> arg) {
        D result_bits = bits_.fetch_sub(arg.to_bits(), std::memory_order_acq_rel);
        return fixed_t<Q, D>(result_bits);
    }
    
    // 原子比较交换
    bool compare_exchange(fixed_t<Q, D>& expected, const fixed_t<Q, D> desired) {
        D expected_bits = expected.to_bits();
        bool success = bits_.compare_exchange_weak(expected_bits, 
            desired.to_bits(),
            std::memory_order_acq_rel);
        expected = fixed_t<Q, D>::from_bits(expected_bits);
        return success;
    }
    
private:
    std::atomic<D> bits_{static_cast<D>(0)};
};

template<typename T>
struct [[nodiscard]] CellBase<Angular<T>> {
    CellBase(const Angular<T> initial_value){
        set(initial_value);
    }

    CellBase(){;}

    void set(const Angular<T>& val) {
        inner_.set(val.to_turns());
    }
    
    Angular<T> get() const {
        return Angular<T>::from_turns(inner_.get());
    }

    auto & inner() {
        return inner_;
    }
    
private:
    CellBase<T> inner_;
};

template<typename T>
requires (std::is_enum_v<T>)
struct [[nodiscard]] CellBase<T>{
    using D = std::underlying_type_t<T>;
    using Inner = CellBase<D>;

    CellBase(const T initial_value):
        inner_(std::bit_cast<D>(initial_value)){;}

    CellBase(){;}

    void set(const T& val) {
        inner_.set(std::bit_cast<D>(val));
    }
    
    [[nodiscard]] T get() const {
        return std::bit_cast<T>(inner_.get());
    }
    
    auto & inner() {
        return inner_;
    }
private:
    CellBase<D> inner_;
};


// 主 Cell 模板
template<typename T>
struct [[nodiscard]] Cell : public CellBase<T> {
    using Self = Cell;
    using Base = CellBase<T>;
    using Base::set;
    using Base::get;

    Cell(const T initial_value):
        Base(initial_value){;}

    Cell(): Base(){;}
    
    Cell & operator =(const Cell &) = delete;
    Cell & operator =(Cell &&) = delete;

    #if 0
    // 便捷的赋值操作符
    Cell& operator=(const T& val) {
        set(val);
        return *this;
    }
    
    // 隐式转换到 T（只读）
    operator T() const {
        return get();
    }
    #endif

    Probe<T> probe() const {
        return Probe<T>(this);
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << self.get();
    }
};

// 探针类 - 用于只读访问
template<typename T>
struct [[nodiscard]] Probe {
    using Self = Probe;

    static Probe null(){
        return Self(nullptr);
    }

    [[nodiscard]] T get() const {
        if(cell_ == nullptr) [[unlikely]]
            PANIC{"try to get a null cell"};
        return cell_->get();
            // return Zero
    }

    [[nodiscard]] Self clone() const {
        return Self(cell_);
    }

    [[nodiscard]] Self probe() const {
        return Self(cell_);
    }


    // 复制构造和赋值
    Probe(const Probe & other){
        cell_ = other.cell_;
    }
    Probe& operator=(const Probe & other){
        cell_ = other.cell_;
        return *this;
    }

    Probe(Probe && other){
        cell_ = other.cell_;
    }
    Probe& operator=(Probe && other){
        cell_ = other.cell_;
        return *this;
    }
private:
    const Cell<T>* cell_;


    explicit Probe(const Cell<T> * cell) : 
        cell_(cell) {}

    friend OutputStream & operator <<(OutputStream & os, const Probe & self){
        return os << self.get();
    }

    friend class Cell<T>;
};

//CTAD
template<typename T>
Cell(T) -> Cell<T>;

template<typename T>
Probe(T *) -> Probe<T>;


template<typename T>
struct Mailbox{
    Result<void, void> try_store(const T & value){ 
        if(has_value_){
            return Err();
        }
        value_ = value;
        return Ok();
    }

    void force_store(const T & value){ 
        value_ = value;
    }

    bool has_value() const{ 
        return has_value_;
    }

    Option<T> take(){ 
        if(not has_value_)
            return None;
        return Some(value_);
    }

private:
    T value_;
    bool has_value_ = false;
};

}