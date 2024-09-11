#pragma once
#include "YUME/Core/base.h"
#include "reference_counter.h"

// std
#include <cstdio>
#include <format>
#include <string>


namespace YUME
{
	#define YM_NODISCARD [[nodiscard]]

	#define YM_REFERENCE_DEBUG
	#if defined(YM_DEBUG) && defined(YM_REFERENCE_DEBUG)
		#define YM_REFERENCE_LOG(...) YM_CORE_TRACE(__VA_ARGS__)
	#else
		#define YM_REFERENCE_LOG(...)
	#endif

	class YM_API RefCount
	{
		public:
			RefCount();
			~RefCount();

			inline bool IsReferenced() const
			{
				return m_RefcountInit.Get() < 1;
			}
			bool InitRef();

			// Returns false if refcount is at zero and didn't get increased
			bool reference();
			bool unreference();

			bool weakReference();
			bool weakUnreference();

			int GetReferenceCount() const;
			int GetWeakReferenceCount() const;

		private:
			ReferenceCounter m_Refcount;
			ReferenceCounter m_RefcountInit;
			ReferenceCounter m_WeakRefcount;
	};

	template <class T>
	class YM_API Reference
	{
		public:
			constexpr Reference() noexcept {}
			constexpr Reference(std::nullptr_t) noexcept {}

			explicit Reference(T* p_Ptr) noexcept
			{
				YM_REFERENCE_LOG("Initialize ref")

				if (p_Ptr)
				{
					std::string pointerStr = std::format("0x{:016x}", reinterpret_cast<uintptr_t>(p_Ptr));
					YM_REFERENCE_LOG("\t _Ptr: {}", pointerStr)

					refPointer(p_Ptr);
				}
			}


			Reference(const Reference<T>& p_Other) noexcept
			{
				ref(p_Other);
			}

			Reference(Reference<T>&& p_Rhs) noexcept
			{
				ref(p_Rhs);
			}

			template <typename U>
			inline Reference(const Reference<U>& p_Moving) noexcept
			{
				U* movingPtr = p_Moving.get();

				auto castPointer = static_cast<T*>(movingPtr);

				unref();

				if (castPointer != nullptr)
				{
					if (p_Moving.get() == m_Ptr)
						return;

					if (p_Moving.GetCounter() && p_Moving.get())
					{
						m_Ptr = castPointer;
						m_Counter = p_Moving.GetCounter();
						m_Counter->reference();
					}
				}
				else
				{
					YM_CORE_ERROR("Failed to cast Reference")
				}
			}

			~Reference() noexcept
			{
				unref();
			}

			// Access to smart pointer state
			YM_NODISCARD T* get() const noexcept
			{
				return m_Ptr;
			}

			inline RefCount* GetCounter() const
			{
				return m_Counter;
			}

			inline T* release() noexcept
			{
				T* tmp = nullptr;

				if (m_Counter->unreference())
				{
					delete m_Counter;
					m_Counter = nullptr;
				}

				std::swap(tmp, m_Ptr);
				m_Ptr = nullptr;

				return tmp;
			}

			inline void reset(T* p_Ptr = nullptr)
			{
				unref();

				m_Ptr = p_Ptr;
				m_Counter = nullptr;

				if (m_Ptr != nullptr)
				{
					m_Counter = new RefCount();
					m_Counter->InitRef();
				}
			}

			inline Reference& operator=(const Reference& p_Rhs) noexcept
			{
				ref(p_Rhs);
				return *this;
			}

			inline Reference& operator=(Reference&& p_Rhs) noexcept
			{
				ref(p_Rhs);
				return *this;
			}

			inline Reference& operator=(T* p_NewData) noexcept
			{
				reset(p_NewData);
				return *this;
			}

			template <typename U>
			inline Reference& operator=(const Reference<U>& p_Moving) noexcept
			{
				U* movingPtr = p_Moving.get();

				auto castPointer = dynamic_cast<T*>(movingPtr);

				unref();

				if (castPointer != nullptr)
				{
					if (p_Moving.GetCounter() && p_Moving.get())
					{
						m_Ptr = p_Moving.get();
						m_Counter = p_Moving.GetCounter();
						m_Counter->reference();
					}
				}
				else
				{
					YM_CORE_ERROR("Failed to cast Reference")
				}

				return *this;
			}

			explicit inline operator T* () const noexcept
			{
				return get();
			}

			inline Reference& operator=(std::nullptr_t)
			{
				reset();
				return *this;
			}

			YM_NODISCARD T& operator*() const noexcept
			{
				return *get();
			}

			// Const correct access owned object
			YM_NODISCARD T* operator->() const noexcept
			{
				if (!m_Ptr || !m_Counter)
				{
					YM_CORE_CRITICAL("An error has encountered!")
				}

				std::string pointerStr = std::format("0x{:016x}", reinterpret_cast<uintptr_t>(m_Ptr));
				YM_CORE_TRACE("pointing to ptr: {}", pointerStr)
				return get();
			}
			
			YM_NODISCARD T& operator[](int p_Index) const noexcept
			{
				return get()[p_Index];
			}



			inline explicit constexpr operator bool() const
			{
				return m_Ptr != nullptr;
			}

			inline constexpr bool operator==(const T* p_Ptr) const
			{
				return m_Ptr == p_Ptr;
			}
			inline constexpr bool operator!=(const T* p_Ptr) const
			{
				return m_Ptr != p_Ptr;
			}
			inline constexpr bool operator<(const Reference<T>& p_Rhs) const
			{
				return m_Ptr < p_Rhs.m_Ptr;
			}
			inline constexpr bool operator==(const Reference<T>& p_Rhs) const
			{
				return m_Ptr == p_Rhs.m_Ptr;
			}
			inline constexpr bool operator!=(const Reference<T>& p_Rhs) const
			{
				return m_Ptr != p_Rhs.m_Ptr;
			}

			inline void swap(Reference& p_Other) noexcept
			{
				std::swap(m_Ptr, p_Other.m_Ptr);
				std::swap(m_Counter, p_Other.m_Counter);
			}

			template <typename U>
			inline Reference<U> As() const
			{
				return Reference<U>(*this);
			}

		private:
			inline void ref(const Reference& p_From)
			{
				if (p_From.m_Ptr == m_Ptr)
					return;

				std::string pointerStr = std::format("0x{:016x}", reinterpret_cast<uintptr_t>(p_From.m_Ptr));
				YM_REFERENCE_LOG("Reference ptr {}", pointerStr)

				unref();

				m_Counter = nullptr;
				m_Ptr = nullptr;

				if (p_From.GetCounter() && p_From.get())
				{
					m_Ptr = p_From.get();
					m_Counter = p_From.GetCounter();
					m_Counter->reference();
				}
			}

			inline void refPointer(T* p_Ptr)
			{
				YM_CORE_ASSERT(p_Ptr, "Creating ref with nullptr")

				m_Ptr = p_Ptr;
				m_Counter = new RefCount();
				m_Counter->InitRef();
			}

			inline void unref()
			{
				if (m_Counter != nullptr)
				{
					std::string pointerStr = std::format("0x{:016x}", reinterpret_cast<uintptr_t>(m_Ptr));

					if (m_Counter->unreference())
					{
						YM_REFERENCE_LOG("No long reference, destroying ptr {}...", pointerStr)

						delete m_Ptr;

						if (m_Counter->GetWeakReferenceCount() == 0)
						{
							delete m_Counter;
						}

						m_Ptr = nullptr;
						m_Counter = nullptr;
					}
				}
			}

		private:
			RefCount* m_Counter = nullptr;
			T* m_Ptr = nullptr;
	};

	template <class T>
	class YM_API WeakReference
	{
		public:
			WeakReference() noexcept
				: m_Ptr(nullptr)
				, m_Counter(nullptr)
			{
			}

			WeakReference(std::nullptr_t) noexcept
				: m_Ptr(nullptr)
				, m_Counter(nullptr)
			{
			}

			WeakReference(const WeakReference<T>& p_Rhs) noexcept
				: m_Ptr(p_Rhs.m_Ptr)
				, m_Counter(p_Rhs.m_Counter)
			{
				AddRef();
			}

			explicit WeakReference(T* p_Ptr) noexcept
				: m_Ptr(p_Ptr)
			{
				YM_CORE_ASSERT(p_Ptr, "Creating weak ptr with nullptr")

				m_Counter = new RefCount();
				m_Counter->weakReference();
			}

			template <class U>
			WeakReference(const WeakReference<U>& p_Rhs) noexcept
				: m_Ptr(p_Rhs.m_Ptr)
				, m_Counter(p_Rhs.m_Counter)
			{
				AddRef();
			}

			WeakReference(const Reference<T>& p_Rhs) noexcept
				: m_Ptr(p_Rhs.get())
				, m_Counter(p_Rhs.GetCounter())
			{
				AddRef();
			}

			~WeakReference() noexcept
			{
				if (m_Counter->weakUnreference())
				{
					delete m_Ptr;
				}
			}

			void AddRef()
			{
				m_Counter->weakReference();
			}

			bool Expired() const
			{
				return m_Counter ? m_Counter->GetReferenceCount() <= 0 : true;
			}

			Reference<T> lock() const
			{
				if (Expired())
					return Reference<T>();
				else
					return Reference<T>(m_Ptr);
			}

			inline T* operator->() const
			{
				return &*m_Ptr;
			}
			inline T& operator*() const
			{
				return *m_Ptr;
			}

			inline T& operator[](int p_Index)
			{
				YM_CORE_ASSERT(m_Ptr != nullptr)
				return m_Ptr[p_Index];
			}

			inline explicit operator bool() const
			{
				return m_Ptr != nullptr;
			}
			inline bool operator==(const T* p_Ptr) const
			{
				return m_Ptr == p_Ptr;
			}
			inline bool operator!=(const T* p_Ptr) const
			{
				return m_Ptr != p_Ptr;
			}
			inline bool operator<(const WeakReference<T>& p_Rhs) const
			{
				return m_Ptr < p_Rhs.m_Ptr;
			}
			inline bool operator==(const WeakReference<T>& p_Rhs) const
			{
				return m_Ptr == p_Rhs.m_Ptr;
			}
			inline bool operator!=(const WeakReference<T>& p_Rhs) const
			{
				return m_Ptr != p_Rhs.m_Ptr;
			}

		private:
			T* m_Ptr;
			RefCount* m_Counter = nullptr;
	};

	template <class T>
	class YM_API Unique
	{
		public:
			explicit Unique(std::nullptr_t)
				: m_Ptr(nullptr)
			{
			}

			Unique(T* ptr = nullptr)
			{
				m_Ptr = ptr;
			}

			template <class U>
			explicit Unique(U* p_Ptr)
			{
				m_Ptr = dynamic_cast<T*>(p_Ptr);
			}

			~Unique()
			{
				delete m_Ptr;
			}

			Unique(const Unique&) = delete;
			Unique& operator=(const Unique&) = delete;

			inline Unique(Unique&& p_Moving) noexcept
			{
				p_Moving.swap(*this);
			}

			inline Unique& operator=(Unique&& p_Moving) noexcept
			{
				p_Moving.swap(*this);
				return *this;
			}

			template <class U>
			explicit inline Unique(Unique<U>&& p_Moving)
			{
				Unique<T> tmp(p_Moving.release());
				tmp.swap(*this);
			}

			template <class U>
			inline Unique& operator=(Unique<U>&& p_Moving)
			{
				Unique<T> tmp(p_Moving.release());
				tmp.swap(*this);
				return *this;
			}

			inline Unique& operator=(std::nullptr_t)
			{
				reset();
				return *this;
			}

			// Const correct access unique object
			inline T* operator->() const
			{
				return &*m_Ptr;
			}

			T& operator*() const
			{
				return *m_Ptr;
			}

			T& operator[](int p_Index)
			{
				YM_CORE_ASSERT(m_Ptr != nullptr)
				return m_Ptr[p_Index];
			}

			// Access to smart pointer state
			inline T* get() const
			{
				return m_Ptr;
			}
			explicit operator bool() const
			{
				return m_Ptr;
			}

			// Modify object state
			inline T* release()
			{
				T* result = nullptr;
				std::swap(result, m_Ptr);
				return result;
			}

			inline void reset()
			{
				T* tmp = release();
				delete tmp;
			}

			inline void swap(Unique& src) noexcept
			{
				std::swap(m_Ptr, src.m_Ptr);
			}

			template <class U>
			inline Unique<U> As() const
			{
				return Unique<U>(*this);
			}

		private:
			T* m_Ptr = nullptr;
	};

	template <class T>
	void swap(Unique<T>& p_Lhs, Unique<T>& p_Rhs) noexcept
	{
		p_Lhs.swap(p_Rhs);
	}

//#define CUSTOM_REFERENCE
#ifdef CUSTOM_REFERENCE

	template <class T>
	using Scope = Unique<T>;

	template <class T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... p_Args)
	{
		auto ptr = new T(std::forward<Args>(p_Args)...);
		return Scope<T>(ptr);
	}

	template <class T>
	using Ref = Reference<T>;

	template <class T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... p_Args)
	{
		auto ptr = new T(std::forward<Args>(p_Args)...);
		return Ref<T>(ptr);
	}

	template <class T>
	using WeakRef = WeakReference<T>;

#else

	template<class T>
	class YM_API Scope : public std::unique_ptr<T>
	{
		public:
			using std::unique_ptr<T>::unique_ptr;

			template<class U>
			Scope(std::unique_ptr<U>&& p_Other)
				: std::unique_ptr<T>(std::move(p_Other)) {}

			template<class U>
			Scope<U> As() const
			{
				//YM_CORE_ASSERT(std::is_base_of<U, T>::value)
				return Scope<U>(std::dynamic_pointer_cast<U>(*this));
			}
	};

	template<class T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... p_Args)
	{
		return Scope<T>(std::make_unique<T>(std::forward<Args>(p_Args)...));
	}

	template<class T>
	class YM_API Ref : public std::shared_ptr<T> 
	{
		public:
			using std::shared_ptr<T>::shared_ptr;

			template<class U>
			Ref(const std::shared_ptr<U>& p_Other)
				: std::shared_ptr<T>(p_Other) {}

			template<class U>
			Ref<U> As() const
			{
				//YM_CORE_ASSERT(std::is_base_of<U, T>::value)
				return Ref<U>(std::dynamic_pointer_cast<U>(*this));
			}
	};

	template<class T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... p_Args)
	{
		return Ref<T>(std::make_shared<T>(std::forward<Args>(p_Args)...));
	}

	template<class T>
	using WeakRef = std::weak_ptr<T>;

#endif

} // YUME

#ifdef CUSTOM_REFERENCE
namespace std
{
	template <class T>
	struct hash<YUME::Reference<T>>
	{
		size_t operator()(const YUME::Reference<T>& p_Value) const
		{
			return hash<T*>()(p_Value.get());
		}
	};
}
#endif
