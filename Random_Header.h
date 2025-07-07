#pragma once
#include <random> 

namespace Random //Um Zufallszahlen erzeugen zu können
{
	//Globales mt19937 Objekt. Erzeugt 64bit int Zufallszahl mit generator().
	inline std::random_device rd{};		//random_device: Zufälliger Seed vom Computer bestimmt.
	inline std::seed_seq seed{ rd(),rd(), rd(), rd(), rd(), rd(), rd(), rd() }; //Erzeugt neuen Seed mit acht Zufallszahlen von rd.
	inline std::mt19937_64 generator{ seed };

	//Funktion für zufälliges uniformverteiltes int
	inline int randInt(int min, int max)
	{
		return std::uniform_int_distribution{ min, max }(generator);
	}

	//Funktion für zufälliges uniformverteiltes double
	inline double randDouble(double min = 0.0, double max = 1.0)	//Default Option: Uniformverteilt zwischen 0 und 1.
	{
		return std::uniform_real_distribution{ min, max }(generator);
	}

//Gauss-Verteilung
	inline constexpr double pi{ 3.14159265358979323846 };
	inline constexpr double e{  2.71828182845904523536 };
	inline double pdfNormal(double x, double mu = 0.0, double sigma = 1.0)
	{
		return std::pow(2.0 * pi * std::pow(sigma, 2.0), -0.5) * std::pow(e, -0.5 * std::pow((x - mu) / sigma, 2.0));
	}
}