#ifndef UQUAD_BASE_DELEGATE_H
#define UQUAD_BASE_DELEGATE_H

#include "Config.h"

namespace uquad
{
namespace base
{
	template <typename T>
	class Delegate
	{
	public:
		friend class Subscription;
		friend class ScopedSubscription;

		typedef Delegate<T> self_t;
		typedef unordered_set<T*> subscribers_t;

		class Subscription
		{
		public:
			friend class ScopedSubscription;

			Subscription()
				: m_Delegate()
				, m_Subscriber()
			{

			}

			Subscription(Subscription const &that)
				: m_Delegate(that.m_Delegate)
				, m_Subscriber(that.m_Subscriber)
			{

			}

			Subscription(self_t *d, typename subscribers_t::iterator s)
				: m_Delegate(d)
				, m_Subscriber(s)
			{
			}

			~Subscription()
			{

			}

			inline operator bool() const
			{
				return m_Delegate;
			}

			inline bool operator==(Subscription const &rhs) const
			{
				return m_Delegate == rhs.m_Delegate && (!m_Delegate || m_Subscriber == rhs.m_Subscriber);
			}

			inline bool operator!=(Subscription const &rhs) const
			{
				return m_Delegate != rhs.m_Delegate || (m_Delegate && m_Subscriber != rhs.m_Subscriber);
			}

			Subscription& operator =(Subscription const &rhs)
			{
				m_Delegate = rhs.m_Delegate;
				m_Subscriber = rhs.m_Subscriber;
				return *this;
			}

			bool unsubscribe()
			{
				if(m_Delegate)
				{
					m_Delegate->remove(m_Subscriber);
					m_Delegate = 0;
					return true;
				}
				return false;
			}

		private:
			self_t *m_Delegate;
			typename subscribers_t::iterator m_Subscriber;
		};

		class ScopedSubscription
		{
		public:
			ScopedSubscription(Subscription const &s)
				: m_Subscription(s)
			{
			}

			~ScopedSubscription()
			{
				unsubscribe();
			}

			inline operator bool() const
			{
				return m_Subscription;
			}

			inline bool operator==(Subscription const &rhs) const
			{
				return m_Subscription == rhs;
			}

			inline bool operator!=(Subscription const &rhs) const
			{
				return m_Subscription != rhs;
			}

			ScopedSubscription& operator =(Subscription const &s)
			{
				if(m_Subscription)
				{
					if(m_Subscription == s)
						return *this;
					m_Subscription.unsubscribe();
				}
				m_Subscription = s;
				return *this;
			}

			bool unsubscribe()
			{
				return m_Subscription.unsubscribe();
			}

		private:
			Subscription m_Subscription;
		};

		Delegate()
			: m_Sync()
			, m_Subscribers()

		{

		}

		~Delegate()
		{

		}

		Subscription operator+=(T *s) const
		{
			std::pair<typename subscribers_t::iterator, bool> ib = const_cast<self_t*>(this)->add(s);
			if(!ib.second) return Subscription();

			return Subscription(
				const_cast<self_t*>(this),
				ib.first
			);
		}

		void operator-=(T *s) const
		{
			const_cast<self_t*>(this)->remove(s);
		}

		template <typename Delegate, typename ...Args>
		void call(Delegate delegate,  Args &&... args)
		{
			lock_guard<fast_mutex> lock(m_Sync);
			std::for_each(m_Subscribers.begin(), m_Subscribers.end(), [=](T *subscriber)
			{
				(subscriber->*delegate)(args...);
			});
		}

	private:

		std::pair<typename subscribers_t::iterator, bool> add(T *s) const
		{
			lock_guard<fast_mutex> lock(m_Sync);
			return m_Subscribers.insert(s);
		}

		void remove(T *s)
		{
			lock_guard<fast_mutex> lock(m_Sync);
			m_Subscribers.erase(s);
		}

		void remove(typename subscribers_t::iterator i)
		{
			lock_guard<fast_mutex> lock(m_Sync);
			m_Subscribers.erase(i);
		}

	private:
		mutable fast_mutex m_Sync;
		mutable subscribers_t m_Subscribers;
	};
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_DELEGATE_H
