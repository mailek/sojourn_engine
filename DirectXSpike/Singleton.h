#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	Singleton.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
template <typename T/*concrete type*/, typename A/*abstract type*/>
class SingleThreadSingleton
{
public:
	static A* GetInstance()
	{
		if( _mock_instance )
			return _mock_instance;
		if( _instance == NULL )
		{
			_instance = new T();
		}

		return static_cast<A*>(_instance);
	}

	static void DestroySingleton()
	{
		if( _instance != NULL )
		{
			delete _instance;
			_instance = NULL;
		}
	}

	static void SetMockInstance( A *p)
	{
		_mock_instance = p;
	}

private:
	static T*		_instance;
	static A*		_mock_instance;
};

template <typename T, typename A> T* SingleThreadSingleton<T, A>::_instance = NULL;
template <typename T, typename A> A* SingleThreadSingleton<T, A>::_mock_instance = NULL;