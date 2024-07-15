#pragma once

#include "YUME/Core/base.h"

#include <chrono>



namespace YUME
{
	class YM_API Timer
	{
		public:
            Timer() = default;

            void Start()
            {
                m_Start = std::chrono::high_resolution_clock::now();
                m_Running = true;
            }

            void Stop()
            {
                m_End = std::chrono::high_resolution_clock::now();
                m_Running = false;
            }

            double Elapsed() const
            {
                std::chrono::time_point<std::chrono::high_resolution_clock> end;

                if (m_Running)
                    end = std::chrono::high_resolution_clock::now();
                else
                    end = m_End;

                return std::chrono::duration<double>(end - m_Start).count();
            }

		private:
            std::chrono::time_point<std::chrono::high_resolution_clock> m_Start{};
            std::chrono::time_point<std::chrono::high_resolution_clock> m_End{};
			bool m_Running = false;
	};
}