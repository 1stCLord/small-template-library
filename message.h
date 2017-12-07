#pragma once

template <listener>
class message
{
	virtual void call(listener *listener) = 0;
};

template<listener, function, ...>
class std_message : message<listener>
{
	virtual void call(listener *listener) override
	{
		listener->(function)(...);
	}
};