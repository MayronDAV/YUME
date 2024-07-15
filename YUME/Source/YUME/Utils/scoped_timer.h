#pragma once

#include "YUME/Core/base.h"
#include "YUME/Utils/timer.h"


namespace YUME
{
	class YM_PUBLIC ScopedTimer
	{
		public:
			ScopedTimer() { Reset(); }
			~ScopedTimer() { m_Timer.Stop(); }

			void Reset() { m_Timer.Start(); }
			double ElapsedSeconds() const { return m_Timer.Elapsed(); }
			double ElapsedMillis() const { return m_Timer.Elapsed() * 1000.0; }

		private:
			Timer m_Timer{};
		
	};
}