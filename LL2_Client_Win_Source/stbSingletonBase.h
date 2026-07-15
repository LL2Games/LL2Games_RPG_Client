#pragma once
#include <memory>

namespace stb
{
	template <typename T>
		class SingletonBase
		{
		public:
			static T* getInstance()
			{

				if (instance == nullptr)
				{
					instance = new T();
				}

				return instance;
			}

		public:
			SingletonBase() {};
			~SingletonBase() {};

		private:
			static T* instance;

		};

		template <typename T>
		T* SingletonBase<T>::instance = nullptr;

	}
