#pragma once

namespace Core
{
	class Console
	{
	private:
		Console();
		Console(const Console& other) {};
		~Console();

	public:
		static Console* GetInstance();

	private:

	};


}