#ifndef UQUAD_MATH_FIXED_POINT_H
#define UQUAD_MATH_FIXED_POINT_H

#include "Config.h"
#include "Constant.h"

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU)
#   define UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_ATAN2
#   define UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_EXP
#   define UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_COS
#   define UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_SIN
#   define UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_SQRT
#endif

namespace uquad
{
namespace math
{
	template <boost::uint32_t I, boost::uint32_t F > class FixedPoint;

} //namespace math
} //namespace uquad

namespace std {

	template < boost::uint32_t I, boost::uint32_t F >
	uquad::math::FixedPoint<I,F> abs(uquad::math::FixedPoint<I,F> const &);

	template < boost::uint32_t I, boost::uint32_t F >
	uquad::math::FixedPoint<I,F> fabs(uquad::math::FixedPoint<I,F> const &);

	template < boost::uint32_t I, boost::uint32_t F >
	uquad::math::FixedPoint<I,F> ceil(uquad::math::FixedPoint<I,F> const &);

	template < boost::uint32_t I, boost::uint32_t F >
	uquad::math::FixedPoint<I,F> floor(uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> fmod(uquad::math::FixedPoint<I,F> const &, uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> modf(uquad::math::FixedPoint<I,F> const &, uquad::math::FixedPoint<I,F> *);

	

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> sqrt(uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> log(uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> pow(uquad::math::FixedPoint<I,F> const &, uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> exp(uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> atan2(uquad::math::FixedPoint<I,F> const &, uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> sin(uquad::math::FixedPoint<I,F> const &);

	template <boost::uint32_t I, boost::uint32_t F>
	uquad::math::FixedPoint<I,F> cos(uquad::math::FixedPoint<I,F> const &);

} //namespace std

namespace uquad
{
namespace math
{

	namespace detail
	{
		template < unsigned int B >
		struct FixedPointValue { };

		template <>
		struct FixedPointValue< 1 > { typedef boost::int8_t type; };

		template <>
		struct FixedPointValue< 2 > { typedef boost::int16_t type; };

		template <>
		struct FixedPointValue< 3 > { typedef boost::int32_t type; };

		template <>
		struct FixedPointValue< 4 > { typedef boost::int32_t type; };

		template <>
		struct FixedPointValue< 5 > { typedef boost::int64_t type; };

		template <>
		struct FixedPointValue< 6 > { typedef boost::int64_t type; };

		template <>
		struct FixedPointValue< 7 > { typedef boost::int64_t type; };

		template <>
		struct FixedPointValue< 8 > { typedef boost::int64_t type; };


		template < boost::uint32_t V >
		struct pow2
		{
			static boost::int64_t const value=2*pow2<V-1>::value;
		};

		template <>
		struct pow2<0>
		{
			static boost::int64_t const value=1;
		};

		template < bool, typename T1, typename T2 >
		struct select {};

		template < typename T1, typename T2 >
		struct select<true, T1, T2>
		{
			typedef T1 type;
		};

		template < typename T1, typename T2 >
		struct select<false, T1, T2>
		{
			typedef T2 type;
		};



		template < bool >
		struct adjuster { };

		template < >
		struct adjuster<true>
		{
			template < typename T1, typename T2, boost::uint32_t F1, boost::uint32_t F2 >
			static T1 adjust(T2 v)
			{
				typedef typename select<
					(std::numeric_limits<T1>::digits > std::numeric_limits<T2>::digits),
					T1,
					T2
				>::type greater_type;
				return static_cast<T1>(greater_type(v)<<(F1-F2));
			}
		};

		template < >
		struct adjuster<false>
		{
			template < typename T1, typename T2, boost::uint32_t F1, boost::uint32_t F2 >
			static T1 adjust(T2 v)
			{
				typedef typename select<
					(std::numeric_limits<T1>::digits > std::numeric_limits<T2>::digits),
					T1,
					T2
				>::type greater_type;
				return static_cast<T1>(greater_type(v)>>(F2-F1));
			}
		};

		struct fp_explicit_tag {};

	} //namespace detail



	template < boost::uint32_t I, boost::uint32_t F >
	class FixedPoint
	{
		template <boost::uint32_t, boost::uint32_t> friend class FixedPoint;

	public:
		static boost::uint32_t const IS=I;
		static boost::uint32_t const FS=F;

		static boost::uint32_t const int_bits_count=I+F+1;
		static boost::uint32_t const int_bytes_count=(int_bits_count+7)/8;
		typedef typename detail::FixedPointValue<int_bytes_count>::type value_type;

		inline FixedPoint()
			: m_value(0)
		{
		}

		inline explicit FixedPoint(value_type v, detail::fp_explicit_tag)
			: m_value(v)
		{
		}

#define UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(T)\
		inline FixedPoint(T v)\
			: m_value(static_cast<value_type>(v)<<F)\
		{\
		}

		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::uint8_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::uint16_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::uint32_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::uint64_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::int8_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::int16_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::int32_t)
		UQUAD_MATH_FIXED_POINT_INTEGRAL_CONSTRUCTOR(boost::int64_t)



		inline FixedPoint(float v)
			: m_value(static_cast<value_type>(v*detail::pow2<F>::value+(v>=0.0f?0.5f:-0.5f)))
		{
		}

		inline FixedPoint(double v)
			: m_value(static_cast<value_type>(v*detail::pow2<F>::value+(v>=0.0?0.5:-0.5)))
		{
		}

		inline FixedPoint(long double v)
			: m_value(static_cast<value_type>(v*detail::pow2<F>::value+(v>=0.0?0.5:-0.5)))
		{
		}

		inline FixedPoint(FixedPoint const& that)
			: m_value(that.m_value)
		{
		}

		template< boost::uint32_t I2, boost::uint32_t F2 >
		inline FixedPoint(FixedPoint< I2, F2 > const& that)
			: m_value( detail::adjuster< (F>F2) >::
				template adjust< value_type, typename FixedPoint< I2, F2 >::value_type, F, F2>(that.m_value)
			)
		{ 
		}

		inline FixedPoint& operator =(FixedPoint const& rhs)
		{
			m_value=rhs.m_value;
			return *this;
		}

		template< boost::uint32_t I2, boost::uint32_t F2 >
		inline FixedPoint& operator =(FixedPoint< I2, F2 > const& rhs)
		{
			m_value=FixedPoint(rhs).m_value;
			return *this;
		}

		inline bool operator <(FixedPoint const& rhs) const
		{
			return m_value<rhs.m_value; 
		}

		inline bool operator <=(FixedPoint const& rhs) const
		{
			return m_value<=rhs.m_value; 
		}

		inline bool operator >(FixedPoint const& rhs) const
		{
			return m_value>rhs.m_value; 
		}

		inline bool operator >=(FixedPoint const& rhs) const
		{
			return m_value>=rhs.m_value; 
		}

		inline bool operator ==(FixedPoint const& rhs) const
		{
			return m_value==rhs.m_value; 
		}

		inline bool operator !=(FixedPoint const& rhs) const
		{
			return m_value!=rhs.m_value; 
		}

		inline bool operator !() const
		{
			return m_value==0;
		}

		inline FixedPoint operator -() const
		{
			return FixedPoint(-m_value, detail::fp_explicit_tag());
		}

		inline FixedPoint& operator ++()
		{
			m_value+=detail::pow2<F>::value;
			return *this;
		}

		inline FixedPoint& operator --()
		{
			m_value-=detail::pow2<F>::value;
			return *this;
		}

		inline FixedPoint& operator +=(FixedPoint const& rhs)
		{
			m_value+=rhs.m_value;
			return *this;
		}

		inline FixedPoint operator +(FixedPoint const& rhs) const
		{
			return FixedPoint(m_value+rhs.m_value, detail::fp_explicit_tag());
		}

		inline FixedPoint& operator -=(FixedPoint const& rhs)
		{
			m_value-=rhs.m_value;
			return *this;
		}

		inline FixedPoint operator -(FixedPoint const& rhs) const
		{
			return FixedPoint(m_value-rhs.m_value, detail::fp_explicit_tag());
		}

		inline FixedPoint& operator *=(FixedPoint const& rhs)
		{
			m_value=(typename FixedPoint<2*I, 2*F>::value_type(m_value)*rhs.m_value)>>F;
			return *this;
		}

		inline FixedPoint operator *(FixedPoint const& rhs) const
		{
			return FixedPoint(
				(typename FixedPoint<2*I, 2*F>::value_type(m_value)*rhs.m_value)>>F,
				detail::fp_explicit_tag()
				);
		}

		inline FixedPoint& operator /=(FixedPoint const& rhs)
		{
			m_value=static_cast<value_type>(
                (typename FixedPoint<2*I, 2*F>::value_type(m_value)<<F)/
                typename FixedPoint<2*I, 2*F>::value_type(rhs.m_value)
            );
			return *this;
		}

		inline FixedPoint operator /(FixedPoint const& rhs) const
		{
			return FixedPoint(
                static_cast<value_type>(
                    (typename FixedPoint<2*I, 2*F>::value_type(m_value)<<F)/
                    typename FixedPoint<2*I, 2*F>::value_type(rhs.m_value)
                ),
                detail::fp_explicit_tag()
			);
		}

		inline FixedPoint& operator >>=(boost::uint32_t b)
		{
			m_value>>=b;
			return *this;
		}

		inline FixedPoint& operator <<=(boost::uint32_t b)
		{
			m_value<<=b;
			return *this;
		}

		inline FixedPoint operator >>(boost::uint32_t b) const
		{
			return FixedPoint(m_value>>b, detail::fp_explicit_tag());
		}

		inline FixedPoint operator <<(boost::uint32_t b) const
		{
			return FixedPoint(m_value<<b, detail::fp_explicit_tag());
		}

		inline operator char() const { return static_cast<char>(m_value>>F); }

		inline operator signed char() const { return static_cast<signed char>(m_value>>F); }

		inline operator unsigned char() const { return static_cast<unsigned char>(m_value>>F); }

		inline operator short() const { return static_cast<short>(m_value>>F); }

		inline operator unsigned short() const { return static_cast<unsigned short>(m_value>>F); }

		inline operator int() const { return static_cast<int>(m_value>>F); }

		inline operator unsigned int() const { return static_cast<unsigned int>(m_value>>F); }

		inline operator long() const { return static_cast<long>(m_value>>F); }

		inline operator unsigned long() const { return static_cast<unsigned long>(m_value>>F); }

		inline operator long long() const { return static_cast<long long>(m_value>>F); }

		inline operator unsigned long long() const { return static_cast<unsigned long long>(m_value>>F); }

		inline operator bool() const { return m_value?true:false; }

		inline operator float() const { return static_cast<float>(m_value)/detail::pow2<F>::value; }

		inline operator double() const { return static_cast<double>(m_value)/detail::pow2<F>::value; }

		inline operator long double() const { return static_cast<long double>(m_value)/detail::pow2<F>::value; }


		inline value_type get() const { return m_value; }

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::abs(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::fabs(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::ceil(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::floor(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::fmod(FixedPoint<I2,F2> const &, FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::modf(FixedPoint<I2,F2> const &, FixedPoint<I2,F2> *);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::sqrt(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::log(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::pow(FixedPoint<I2,F2> const &, FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::exp(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::atan2(FixedPoint<I2,F2> const &, FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::sin(FixedPoint<I2,F2> const &);

		template < boost::uint32_t I2, boost::uint32_t F2 >
		friend FixedPoint<I2,F2> std::cos(FixedPoint<I2,F2> const &);


	private:
		value_type		m_value;
	};

} //namespace math
} //namespace uquad

namespace std
{
	template< boost::uint32_t I, boost::uint32_t F>
	class numeric_limits< uquad::math::FixedPoint<I, F> >
	{
	public:
		typedef uquad::math::FixedPoint<I, F> fp_type;

		static const float_denorm_style has_denorm=denorm_absent;
		static const bool has_denorm_loss=false;
		static const bool has_infinity=false;
		static const bool has_quiet_NaN=false;
		static const bool has_signaling_NaN=false;
		static const bool is_bounded=true;
		static const bool is_exact=true;
		static const bool is_iec559=false;
		static const bool is_integer=false;
		static const bool is_modulo=false;
		static const bool is_signed=std::numeric_limits<typename fp_type::value_type>::is_signed;
		static const bool is_specialized=true;
		static const bool tinyness_before=false;
		static const bool traps=false;
		static const float_round_style round_style=round_toward_zero;
		static const int digits=I;
		//static const  digits10=(int)(I*301./1000.+.5);
		static const int max_exponent=0;
		//static const int max_exponent10=0;
		static const int min_exponent=0;
		//static const int min_exponent10=0;
		static const int radix=0;
		static inline fp_type (min)()
		{
			//return fp_type((std::numeric_limits<typename fp_type::value_type>::min)(), uquad::math::detail::fp_explicit_tag());
			return fp_type(1, uquad::math::detail::fp_explicit_tag());
		}

		static inline fp_type (max)()
		{
			return fp_type((std::numeric_limits<typename fp_type::value_type>::max)(), uquad::math::detail::fp_explicit_tag());
		}

		static inline fp_type (epsilon)()
		{
			return fp_type(1, uquad::math::detail::fp_explicit_tag() );
		}

		static inline fp_type (lowest)()
		{
			return fp_type((std::numeric_limits<typename fp_type::value_type>::min)(), uquad::math::detail::fp_explicit_tag());
		}

		static inline fp_type round_error()
		{
			return fp_type(0.5);
		}

		static inline fp_type denorm_min()
		{
			return fp_type(0);
		}

		static inline fp_type infinity()
		{
			return fp_type(0);
		}

		static inline fp_type quiet_NaN()
		{
			return fp_type(0);
		}

		static inline fp_type signaling_NaN()
		{
			return fp_type(0);
		}
	};

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> abs(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(std::abs(x.m_value), umath::detail::fp_explicit_tag());
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> fabs(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;
		return x<umath::constant::zero< umath::FixedPoint<I,F> >()?-x:x;
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> ceil(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(x.m_value & ~(umath::detail::pow2<F>::value-1), umath::detail::fp_explicit_tag())+
			((x.m_value & (umath::detail::pow2<F>::value-1))?
				umath::constant::one< umath::FixedPoint<I,F> >():
				umath::constant::zero< umath::FixedPoint<I,F> >()
			);
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> floor(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(x.m_value & ~(umath::detail::pow2<F>::value-1), umath::detail::fp_explicit_tag());
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> fmod(uquad::math::FixedPoint<I,F> const &x, uquad::math::FixedPoint<I,F> const &y)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(x.m_value % y.m_value, umath::detail::fp_explicit_tag());
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> modf(uquad::math::FixedPoint<I,F> const &x, uquad::math::FixedPoint<I,F> *ptr)
	{
		namespace umath=uquad::math;

		*ptr=umath::FixedPoint<I,F>(x.m_value & ~(umath::detail::pow2<F>::value-1), umath::detail::fp_explicit_tag());
		if(x<umath::constant::zero< umath::FixedPoint<I,F> >())
			*ptr+=umath::constant::one< umath::FixedPoint<I,F> >();

		return umath::FixedPoint<I,F>(
			(x<umath::constant::zero< umath::FixedPoint<I,F> >()?
				-(x.m_value & (umath::detail::pow2<F>::value-1)):
				(x.m_value & (umath::detail::pow2<F>::value-1))
			),
			umath::detail::fp_explicit_tag()
		);
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> sqrt(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_SQRT)
		return umath::FixedPoint<I,F>(std::sqrt(static_cast<float>(x)));
#else

#if 0
		using uquad::math::constant::zero;
		using uquad::math::detail::fp_explicit_tag;

		if(x<zero< FixedPoint<I,F> >())
			return 0;

		typename FixedPoint<2*I, 2*F>::value_type op=
			typename FixedPoint<2*I, 2*F>::value_type(x.m_value)<<(I+1);

		typename FixedPoint<2*I, 2*F>::value_type res=0;
		typename FixedPoint<2*I, 2*F>::value_type one=
			typename FixedPoint<2*I, 2*F>::value_type(1)<<
			(std::numeric_limits<typename FixedPoint<2*I, 2*F>::value_type>::digits-1);

		while(one>op)
			one>>=2;

		while(one!=0)
		{
			if(op>=res+one)
			{
				op=op-(res+one);
				res=res+(one<<1);
			}
			res >>= 1;
			one >>= 2;
		}
		return FixedPoint<I,F>(static_cast< FixedPoint<I, F>::value_type >(res), fp_explicit_tag());
#endif
#endif
	}



	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> log(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(::logf(static_cast<float>(x)));
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> pow(uquad::math::FixedPoint<I,F> const &x, uquad::math::FixedPoint<I,F> const &y)
	{
		namespace umath=uquad::math;
		return umath::FixedPoint<I,F>(std::pow(static_cast<float>(x), static_cast<float>(y)));
	}


	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> exp(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_EXP)

		return umath::FixedPoint<I,F>(std::exp(static_cast<float>(x)));

#else
		static umath::FixedPoint<I,F> const a[]=
		{
			1.64872127070012814684865078781,
			1.28402541668774148407342056806,
			1.13314845306682631682900722781,
			1.06449445891785942956339059464,
			1.03174340749910267093874781528,
			1.01574770858668574745853507208,
			1.00784309720644797769345355976,
			1.00391388933834757344360960390,
			1.00195503359100281204651889805,
			1.00097703949241653524284529261,
			1.00048840047869447312617362381,
			1.00024417042974785493700523392,
			1.00012207776338377107650351967,
			1.00006103701893304542177912060,
			1.00003051804379102429545128481,
			1.00001525890547841394814004262,
			1.00000762942363515447174318433,
			1.00000381470454159186605078771,
			1.00000190735045180306002872525,
			1.00000095367477115374544678825,
			1.00000047683727188998079165439,
			1.00000023841860752327418915867,
			1.00000011920929665620888994533,
			1.00000005960464655174749969329,
			1.00000002980232283178452676169,
			1.00000001490116130486995926397,
			1.00000000745058062467940380956,
			1.00000000372529030540080797502,
			1.00000000186264515096568050830,
			1.00000000093132257504915938475,
			1.00000000046566128741615947508
		};

		umath::FixedPoint<I,F> y(1);
		for(boost::int32_t i=F-1; i>=0; --i)
		{
			if(!(x.m_value & 1<<i))
				y*=a[F-i-1];
		}

		boost::int32_t x_int=std::floor(x);
		if(x_int<0)
		{
			for(boost::int32_t i=1;i<=-x_int;++i)
				y*=umath::constant::inv_e< umath::FixedPoint<I,F> >();
		}
		else
		{
			for(boost::int32_t i=1;i<=x_int;++i)
				y*=umath::constant::e< umath::FixedPoint<I,F> >();
		}
		return y;
#endif
	}



	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> atan2(uquad::math::FixedPoint<I,F> const &y, uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_ATAN2)
		return umath::FixedPoint<I,F>(std::atan2(static_cast<float>(y), static_cast<float>(x)));
#else
#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FIXED_POINT_ATAN2_HIGH_PRECISION)

		FixedPoint abs_y=fabs(y)+FixedPoint(1, detail::fp_explicit_tag());
		FixedPoint angle;

		if(x>=detail::constant::zero<FixedPoint>())
		{
			FixedPoint r=FixedPoint(x.m_value-abs_y.m_value, detail::fp_explicit_tag())/FixedPoint(x.m_value+abs_y.m_value, detail::fp_explicit_tag());
			angle=detail::constant::atan_c1<FixedPoint>()*r*r*r-detail::constant::atan_c2<FixedPoint>()*r+detail::constant::pi_i4<FixedPoint>();
		}
		else
		{
			FixedPoint r=FixedPoint(x.m_value+abs_y.m_value, detail::fp_explicit_tag())/FixedPoint(abs_y.m_value-x.m_value, detail::fp_explicit_tag());
			angle=detail::constant::atan_c1<FixedPoint>()*r*r*r-detail::constant::atan_c2<FixedPoint>()*r+detail::constant::pi_3i4<FixedPoint>();
		}
		return (y<detail::constant::zero<FixedPoint>()?-angle:angle);

#else

		umath::FixedPoint<I,F> abs_y=std::abs(y)+umath::FixedPoint<I,F>(1, umath::detail::fp_explicit_tag());

		umath::FixedPoint<I,F> angle(
			x>=umath::constant::zero< umath::FixedPoint<I,F> >()?
				umath::constant::pi_i4< umath::FixedPoint<I,F> >()-umath::constant::pi_i4< umath::FixedPoint<I,F> >()*
				(umath::FixedPoint<I,F>(x.m_value-abs_y.m_value, umath::detail::fp_explicit_tag())/umath::FixedPoint<I,F>(x.m_value+abs_y.m_value, umath::detail::fp_explicit_tag())):
				umath::constant::pi_3i4< umath::FixedPoint<I,F> >()-umath::constant::pi_i4< umath::FixedPoint<I,F> >()*
				(umath::FixedPoint<I,F>(x.m_value+abs_y.m_value, umath::detail::fp_explicit_tag())/umath::FixedPoint<I,F>(abs_y.m_value-x.m_value, umath::detail::fp_explicit_tag()))
		);
		return (y<umath::constant::zero< umath::FixedPoint<I,F> >()?-angle:angle);

#endif
#endif
	}

	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> sin(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_SIN)

		return umath::FixedPoint<I,F>(std::sin(static_cast<float>(x)));
#else

		static umath::FixedPoint<I,F> const i_2t3=0.16666666666666666666666666666667;
		static umath::FixedPoint<I,F> const i_2t3t4t5=0.00833333333333333333333333333333;
		static umath::FixedPoint<I,F> const i_2t3t4t5t6t7=1.984126984126984126984126984127e-4;

		umath::FixedPoint<I,F> x_=std::fmod(x, umath::constant::pi_2< umath::FixedPoint<I,F> >());
		if(x_>umath::constant::pi< umath::FixedPoint<I,F> >())
			x_-=umath::constant::pi_2< umath::FixedPoint<I,F> >();

		umath::FixedPoint<I,F> xx=x_*x_;

		umath::FixedPoint<I,F> y=-xx*i_2t3t4t5t6t7;
		y+=i_2t3t4t5;
		y*=xx;
		y-=i_2t3;
		y*=xx;
		y+=umath::constant::one< umath::FixedPoint<I,F> >();
		return y*x_;
#endif
	}


	template < boost::uint32_t I, boost::uint32_t F >
	inline uquad::math::FixedPoint<I,F> cos(uquad::math::FixedPoint<I,F> const &x)
	{
		namespace umath=uquad::math;

#if defined(UQUAD_MATH_FIXED_POINT_TRIGONO_USE_FPU_COS)

		return umath::FixedPoint<I,F>(std::cos(static_cast<float>(x)));
#else

		static umath::FixedPoint<I,F> const i_2t3t4=0.04166666666666666666666666666667;
		static umath::FixedPoint<I,F> const i_2t3t4t5t6=0.00138888888888888888888888888889;

		umath::FixedPoint<I,F> x_=std::fmod(x, umath::constant::pi_2< umath::FixedPoint<I,F> >());
		if(x_>umath::constant::pi< umath::FixedPoint<I,F> >())
			x_-=umath::constant::pi_2< umath::FixedPoint<I,F> >();
		umath::FixedPoint<I,F> xx=x_*x_;
		umath::FixedPoint<I,F> y=-xx*i_2t3t4t5t6;
		y+=i_2t3t4;
		y*=xx;
		y-=umath::constant::half< umath::FixedPoint<I,F> >();
		y*=xx;
		return y+umath::constant::one< umath::FixedPoint<I,F> >();
#endif
	}

} //namespace std

namespace uquad
{
namespace math
{

	typedef FixedPoint<15,16> fixp32_t;

} //namespace math
} //namespace uquad

#endif //UQUAD_MATH_FIXED_POINT_H
