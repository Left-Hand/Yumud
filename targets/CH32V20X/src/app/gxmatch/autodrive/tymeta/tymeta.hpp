#pragma once

#include "sys/math/real.hpp"

namespace ymd{

namespace tymeta {

	template<typename T, T ...args>
	struct tyVector
	{
		using Type = List<T, args...>;
	};

	// Specialized
	// kilo gram
	using Mass = tyVector<int, 1, 0, 0, 0, 0, 0, 0>;
	// Meter
	using Length = tyVector<int, 0, 1, 0, 0, 0, 0, 0>;
	// Second
	using Time = tyVector<int, 0, 0, 1, 0, 0, 0, 0>;
	// Ampere
	using Charge = tyVector<int, 0, 0, 0, 1, 0, 0, 0>;
	// Kelvin
	using Temperature = tyVector<int, 0, 0, 0, 0, 1, 0, 0>;
	// Lux
	using Intensity = tyVector<int, 0, 0, 0, 0, 0, 1, 0>;
	// Mol
	using AmountOfSubstance = tyVector<int, 0, 0, 0, 0, 0, 0, 1>;


	template<typename T, typename Dimension>
	struct Quantity
	{
		explicit Quantity(T x) : mValue(x) {};
		T value() const
		{
			return mValue;
		};
	private:
		T mValue;
	};


	template<typename T, typename Dimension>
	Quantity<T, Dimension> operator + (const Quantity<T, Dimension>& x, const Quantity<T, Dimension>& y)
	{
		return Quantity<T, Dimension>(x.value() + y.value());
	}

	template<typename T, typename Dimension>
	Quantity<T, Dimension> operator - (const Quantity<T, Dimension>& x, const Quantity<T, Dimension>& y)
	{
		return Quantity<T, Dimension>(x.value() + y.value());
	}


	template<typename T, T value1, T value2>
	struct PlusImplenment
	{
		using Type = PlusImplenment<T, value1, value2>;
		const static T value = value1 + value2;
	};

	struct Plus
	{
		// Meta function forwards.
		template<typename T, T value1, T value2>
		struct Apply : PlusImplenment<T, value1, value2>{};
	};


	template<typename T, T value1, T value2>
	struct MinusImplenment
	{
		using Type = MinusImplenment<T, value1, value2>;
		const static T value = value1 - value2;
	};

	struct Minus
	{
		// Meta function forwards.
		template<typename T, T value1, T value2>
		struct Apply : MinusImplenment<T, value1, value2>{};
	};

	// Function Signature
	template<typename T1, typename T2, typename MetaFunction>
	struct Transform;


	template<
		typename T,
		T ...args1,
		T ...args2,
		template<T ...args1> class tyVector1,
		template<T ...args2> class tyVector2,
		typename MetaFunction>
	struct Transform<tyVector1<T, args1...>, tyVector2<T, args2...>, MetaFunction>
	{
		using Type = List<T, MetaFunction::template Apply<T, args1, args2>::value...>;
	};


	template<typename T, typename D1, typename D2>
	Quantity<T, typename Transform<D1, D2, Plus>::Type> operator * (const Quantity<T, D1>& x, const Quantity<T, D2> y)
	{
		// Caculate the quantity.
		using Dimension = Transform<D1, D2, Plus>::Type;
		return Quantity<T, Dimension>(x.value() * y.value());
	}

	template<typename T, typename D1, typename D2>
	Quantity<T, typename Transform<D1, D2, Minus>::Type> operator / (const Quantity<T, D1>& x, const Quantity<T, D2> y)
	{
		// Caculate the quantity.
		using Dimension = Transform<D1, D2, Minus>::Type;
		return Quantity<T, Dimension>(x.value() / y.value());
	}

    template<arithmetic T>
    using Mass_t = tyVector<T, 1, 0, 0, 0, 0, 0, 0>;

    template<arithmetic T>
    using Length_t = tyVector<T, 0, 1, 0, 0, 0, 0, 0>;

    template<arithmetic T>
    using Time_t = tyVector<T, 0, 0, 1, 0, 0, 0, 0>;

    template<arithmetic T>
    using Charge_t = tyVector<T, 0, 0, 0, 1, 0, 0, 0>;

    template<arithmetic T>
    using Temperature_t = tyVector<T, 0, 0, 0, 0, 1, 0, 0>;

    template<arithmetic T>
    using Intensity_t = tyVector<T, 0, 0, 0, 0, 1, 0, 0>;

    template<arithmetic T>
    using AmountOfSubstance_t = tyVector<T, 0, 0, 0, 0, 1, 0, 0>;
}


}
