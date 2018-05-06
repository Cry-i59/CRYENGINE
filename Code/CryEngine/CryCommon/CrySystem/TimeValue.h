// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

class CTimeValue
{
public:
	static constexpr int64 TIMEVALUE_PRECISION = 100000; //!< One second.

public:
	void GetMemoryUsage(class ICrySizer* pSizer) const { /*nothing*/ }

	constexpr CTimeValue() : m_value(0) {}
	constexpr CTimeValue(const float fSeconds) : m_value(static_cast<int64>(fSeconds * TIMEVALUE_PRECISION)) {}
	constexpr CTimeValue(const double fSeconds) : m_value(static_cast<int64>(fSeconds * TIMEVALUE_PRECISION)) {}

	//! \param inllValue Positive negative, absolute or relative in 1 second= TIMEVALUE_PRECISION units.
	constexpr CTimeValue(const int64 inllValue) : m_value(inllValue) {}

	//! Copy constructor.
	CTimeValue(const CTimeValue& inValue) : m_value(inValue.m_value) {}

	//! Assignment operator.
	//! \param inRhs Right side.
	CTimeValue& operator=(const CTimeValue& inRhs)
	{
		m_value = inRhs.m_value;
		return *this;
	};

	//! Use only for relative value, absolute values suffer a lot from precision loss.
	constexpr float GetSeconds() const
	{
		return m_value * (1.f / TIMEVALUE_PRECISION);
	}

	//! Get relative time difference in seconds.
	//! Call this on the endTime object: endTime.GetDifferenceInSeconds( startTime );
	float GetDifferenceInSeconds(const CTimeValue& startTime) const
	{
		return (*this - startTime).GetSeconds();
	}

	void SetSeconds(const float infSec)
	{
		m_value = (int64)(infSec * TIMEVALUE_PRECISION);
	}

	void SetSeconds(const double infSec)
	{
		m_value = (int64)(infSec * TIMEVALUE_PRECISION);
	}

	void SetSeconds(const int64 indwSec)
	{
		m_value = indwSec * TIMEVALUE_PRECISION;
	}

	static constexpr CTimeValue CreateFromMilliSeconds(const int64 indwMilliSec)
	{
		return CTimeValue(indwMilliSec * (TIMEVALUE_PRECISION / 1000));
	}

	void SetMilliSeconds(const int64 indwMilliSec)
	{
		*this = CreateFromMilliSeconds(indwMilliSec);
	}

	//! Use only for relative value, absolute values suffer a lot from precision loss.
	constexpr float GetMilliSeconds() const
	{
		return m_value * (1000.f / TIMEVALUE_PRECISION);
	}

	constexpr int64 GetMilliSecondsAsInt64() const
	{
		return m_value * 1000 / TIMEVALUE_PRECISION;
	}

	constexpr int64 GetMicroSecondsAsInt64() const
	{
		return m_value * (1000 * 1000) / TIMEVALUE_PRECISION;
	}

	constexpr int64 GetValue() const
	{
		return m_value;
	}

	void SetValue(int64 val)
	{
		m_value = val;
	}

	constexpr bool IsValid() const
	{
		return m_value != 0;
	}

	//! Useful for periodic events (e.g. water wave, blinking).
	//! Changing TimePeriod can results in heavy changes in the returned value.
	//! \return [0..1]
	float GetPeriodicFraction(const CTimeValue TimePeriod) const
	{
		// todo: change float implement to int64 for more precision
		float fAbs = GetSeconds() / TimePeriod.GetSeconds();
		return fAbs - (int)(fAbs);
	}

	// math operations

	//! Binary subtraction.
	CTimeValue operator-(const CTimeValue& inRhs) const { CTimeValue ret; ret.m_value = m_value - inRhs.m_value; return ret; };

	//! Binary addition.
	CTimeValue operator+(const CTimeValue& inRhs) const { CTimeValue ret; ret.m_value = m_value + inRhs.m_value; return ret;  };

	//! Sign inversion.
	CTimeValue  operator-() const                   { CTimeValue ret; ret.m_value = -m_value; return ret; };

	constexpr CTimeValue& operator+=(const CTimeValue& inRhs) { m_value += inRhs.m_value; return *this; }
	constexpr CTimeValue& operator-=(const CTimeValue& inRhs) { m_value -= inRhs.m_value; return *this; }

	constexpr CTimeValue& operator/=(int inRhs)               { m_value /= inRhs; return *this; }

	// comparison -----------------------

	constexpr bool operator<(const CTimeValue& inRhs) const  { return m_value < inRhs.m_value; };
	constexpr bool operator>(const CTimeValue& inRhs) const  { return m_value > inRhs.m_value; };
	constexpr bool operator>=(const CTimeValue& inRhs) const { return m_value >= inRhs.m_value; };
	constexpr bool operator<=(const CTimeValue& inRhs) const { return m_value <= inRhs.m_value; };
	constexpr bool operator==(const CTimeValue& inRhs) const { return m_value == inRhs.m_value; };
	constexpr bool operator!=(const CTimeValue& inRhs) const { return m_value != inRhs.m_value; };

	AUTO_STRUCT_INFO;

	void GetMemoryStatistics(class ICrySizer* pSizer) const { /*nothing*/ }

private:
	int64 m_value;     //!< Absolute or relative value in 1/TIMEVALUE_PRECISION, might be negative.

	friend class CTimer;
};

constexpr CTimeValue operator"" _days(unsigned long long value)
{
	return CTimeValue(static_cast<double>(value * 86400));
}

constexpr CTimeValue operator"" _days(long double value)
{
	return CTimeValue(static_cast<double>(value) * 86400.0);
}

constexpr CTimeValue operator"" _hours(unsigned long long value)
{
	return CTimeValue(static_cast<double>(value * 3600));
}

constexpr CTimeValue operator"" _hours(long double value)
{
	return CTimeValue(static_cast<double>(value) * 3600.0);
}

constexpr CTimeValue operator"" _minutes(unsigned long long value)
{
	return CTimeValue(static_cast<double>(value * 60));
}

constexpr CTimeValue operator"" _minutes(long double value)
{
	return CTimeValue(static_cast<double>(value) * 60.0);
}

constexpr CTimeValue operator"" _seconds(unsigned long long value)
{
	return CTimeValue(static_cast<double>(value));
}

constexpr CTimeValue operator"" _seconds(long double value)
{
	return CTimeValue(static_cast<double>(value));
}

constexpr CTimeValue operator"" _milliseconds(unsigned long long value)
{
	return CTimeValue::CreateFromMilliSeconds(static_cast<int64>(value));
}