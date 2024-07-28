#pragma once
#include "YUME/Core/base.h"

#include <mutex>


namespace YUME
{
	template<typename T>
	class YM_API Singleton
	{
		public:
			static T& Get()
			{
				if (!m_pInstance)
					m_pInstance = new T();

				return *m_pInstance;
			}

			static void Release()
			{
				if (m_pInstance)
				{
					delete m_pInstance;
					m_pInstance = nullptr;
				}
			}

		protected:
			// Only allow the class to be created and destroyed by itself
			Singleton() = default;
			~Singleton() = default;

			static T* m_pInstance;

		private:
			YM_NONCOPYABLE(Singleton)
	};

	template <class T>
	T* Singleton<T>::m_pInstance = nullptr;



	template <class T>
	class YM_API SingletonInit
	{
		public:
			// Provide global access to the only instance of this class
			static T& Get()
			{
				YM_CORE_ASSERT(m_pInstance == nullptr, "Singleton hasn't been Initialised");
				return *m_pInstance;
			}

			template <typename... TArgs>
			static void Init(TArgs&& ... p_Args)
			{
				YM_CORE_ASSERT(m_pInstance == nullptr, "Calling Init twice");
				m_pInstance = new T(std::forward<TArgs>(p_Args)...);
			}

			// Provide global access to release/delete this class
			static void Release()
			{
				if (m_pInstance)
				{
					delete m_pInstance;
					m_pInstance = nullptr;
				}
			}

		protected:
			// Only allow the class to be created and destroyed by itself
			SingletonInit() = default;
			~SingletonInit() = default;

			static T* m_pInstance;

		private:
			YM_NONCOPYABLE(SingletonInit);
	};

	template <class T>
	T* SingletonInit<T>::m_pInstance = nullptr;



	template<typename T>
	class YM_API ThreadSafeSingleton
	{
		public:

			static T& Get()
			{
				if (!m_pInstance)
				{
					std::scoped_lock<std::mutex> lock(m_pConstructed);
					if (!m_pInstance) // Check to see if a previous thread has already initialised.
						m_pInstance = new T();
				}
				return *m_pInstance;
			}


			static void Release()
			{
				std::scoped_lock<std::mutex> lock(m_pConstructed);
				if (m_pInstance)
				{
					delete m_pInstance;
					m_pInstance = nullptr;
				}
			}

		protected:
			ThreadSafeSingleton() = default;
			~ThreadSafeSingleton() = default;

			static T* m_pInstance;
			static std::mutex m_pConstructed;

		private:
			YM_NONCOPYABLE(ThreadSafeSingleton);
	};

	template <typename T>
	T* ThreadSafeSingleton<T>::m_pInstance = nullptr;
	template <class T>
	std::mutex ThreadSafeSingleton<T>::m_pConstructed;
}