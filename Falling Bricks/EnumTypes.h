#ifndef __ENUMTYPES_H_INCLUDED
#define __ENUMTYPES_H_INCLUDED

template<class T>
T operator++(T &Value)
{
	Value = (T)((size_t)Value + 1);
	return Value;
}

template<class T>
T operator++(T &Value, int)
{
	T ValueCopy = Value;
	Value = (T)((size_t)Value + 1);
	return ValueCopy;
}

template<class T>
T operator--(T &Value)
{
	Value = (T)((size_t)Value - 1);
	return Value;
}

template<class T>
T operator--(T &Value, int)
{
	T ValueCopy = Value;
	Value = (T)((size_t)Value - 1);
	return ValueCopy;
}

#endif // __ENUMTYPES_H_INCLUDED
