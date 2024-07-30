#pragma once
#include "YUME/Core/base.h"
#include <deque>



namespace YUME
{
	class YM_API DeletionQueue
	{
		public:
			void PushFunction(const std::function<void()>& p_Function)
			{
				m_Deletors.push_back(p_Function);
			}

			void Flush()
			{
				for (auto it = m_Deletors.rbegin(); it != m_Deletors.rend(); it++) {
					(*it)(); //call functors
				}

				m_Deletors.clear();
			}

		private:
			std::deque<std::function<void()>> m_Deletors;
	};
}