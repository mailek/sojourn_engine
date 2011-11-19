#pragma once

template <typename T>
class SingleThreadSingleton
{
public:
	static T* GetInstance()
	{
		if( _instance == NULL )
		{
			_instance = new T();
		}

		return _instance;
	}

	static void DestroySingleton()
	{
		if( _instance != NULL )
		{
			delete _instance;
			_instance = NULL;
		}
	}

private:
	static T*		_instance;
};

template <typename T> T* SingleThreadSingleton<T>::_instance = NULL;