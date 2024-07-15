#pragma once

#include "YUME/Core/base.h"



namespace YUME
{
	class YM_API Timestep
	{
		public:
			Timestep(double p_Time = 0.0f)
				: m_Time(p_Time)
			{
			}

			explicit operator float() const { return (float)m_Time; }
			explicit operator double() const { return m_Time; }

			double GetSeconds() const { return m_Time; }
			double GetMilliseconds() const { return m_Time * 1000.0; }


		private:
			double m_Time;
	};
}