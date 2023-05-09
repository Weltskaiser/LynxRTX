#pragma once

#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <glm/vec2.hpp>
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
// #include <chrono>
#include <unistd.h>

static inline constexpr size_t _max_ = 123456789;

auto& getRndGen() {
	//std::cout << "ACCESS RND GEN: seed is " << seed << std::endl;
	static std::random_device rd;  //Will be used to obtain a seed for the random number engine
	static size_t seed = rd();
	static std::mt19937 gen(seed);
	return gen;
}

// x0=seed; a=multiplier; b=increment; m=modulus; n=desired array length; 
std::vector<double> linearRandomGenerator(size_t x0, size_t a, size_t b, size_t m, size_t n) 
{
	auto results = std::vector<double>();
	for (size_t i = 0; i < n; i++) {
		x0 = (a * x0 + b) % m;
		results.emplace_back((double)x0/(double)m);
	}
	return results;
}
auto gener = linearRandomGenerator(546834, 28402348, 12367824, 123456789, 10000000);
size_t gener_i = 0;

size_t scratch_norm(std::vector<double> typ)
{
	// Proba d'être entre x et x+1 du tableau value
	auto proba = std::vector<double>();
	if (typ == std::vector<double>({0, 3})) {
		proba = std::vector<double>({0.261, 0.495, 0.682, 0.817, 0.904, 0.954, 0.980, 0.992});
	}else if (typ == std::vector<double>({0, 2})) {
		proba = std::vector<double>({0.382, 0.682, 0.866, 0.954, 0.987, 0.997});
	} else if (typ == std::vector<double>({0, 0.75})) {
		proba = std::vector<double>({0.817, 0.992});
	} else {
		throw std::invalid_argument("scratch_norm: unexpected typ");
	}
	double u = gener.at(gener_i);
	gener_i += 1;
	// std::cout << "random: scratch_norm " << gener_i << std::endl;
	std::uniform_int_distribution<> distrib(0, _max_);
	// auto& gen = getRndGen();
	// u = (double)distrib(gen)/(double)_max_;
	for (size_t i = 0; i < proba.size(); i++)
		if (u < proba.at(i))
			return i;
	return proba.size();
}

void test(void)
{
	for (auto &typ : std::vector<std::vector<double>>({std::vector<double>({0, 3}), std::vector<double>({0, 2}), std::vector<double>({0, 0.75})})) {
		std::cout << typ.at(0) << " " << typ.at(1) << std::endl;
		auto a = std::vector<size_t>();
		auto b = std::vector<size_t>();
		for (size_t i = 0; i < 12; i++) {
			a.emplace_back(0);
			b.emplace_back(0);
		}
		auto& gen = getRndGen();
		std::normal_distribution<> d {typ.at(0), typ.at(1)};
		for (size_t i = 0; i < 100; i++) {
			a.at(scratch_norm(typ))++;
			// b.at(int(abs(np.random.normal(typ[0], typ[1])))))++;
			b.at((size_t)std::abs(int(d(gen))))++;
		}
		for (size_t i = 0; i < 12; i++) {
			std::cout << a.at(i) << " ";
		}
		std::cout << std::endl;
		for (size_t i = 0; i < 12; i++) {
			std::cout << b.at(i) << " ";
		}
		std::cout << std::endl;
	}
	// throw std::invalid_argument("Fin du test");
}

enum class Terrain
{
	Foret,
	Montagne,
	Prairie,
	Ville,
	Eau
};

void charger_France(std::string carte_input, std::vector<std::vector<Terrain>> &carte)
{
	std::ifstream ifs(carte_input, std::ios::binary);
	if (!ifs.is_open()) {
		throw std::invalid_argument("Invalid filename");
	}
	const std::size_t BUFFER_SIZE = 2048;
	char buffer [BUFFER_SIZE];
	auto lines = std::vector<std::vector<size_t>>();
	while (ifs.read(buffer, 1)) {
		auto row = std::vector<size_t>();
		while (true) {
			row.emplace_back((size_t)('0'+(buffer[0]&0xFF)));
			ifs.read(buffer, 1);
			if ((buffer[0]&0xff) == 0x0A) {
				break;
			}
		}
		lines.emplace_back(row);
	}
	for (auto &line : lines) {
		for (auto &e : line) {
			std::cout << e;
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;


//     with open(carte_input, 'r') as cfl:
//         lignes = cfl.readlines()
//         delta_eau = 5
//         dimensions = [km_par_px*(len(lignes[0])-1)+2*delta_eau, km_par_px*(len(lignes)-1)+2*delta_eau]
//         carte = np.ones([dimensions[1]+2*delta_eau, dimensions[0]+2*delta_eau])*4
//         for j in range(0, dimensions[1]-2*delta_eau, km_par_px):
//             for i in range(0, dimensions[0]-2*delta_eau, km_par_px):
//                 carte[j+delta_eau:j+delta_eau+km_par_px, i+delta_eau:i+delta_eau+km_par_px] = lignes[int(j/km_par_px)][int(i/km_par_px)]
//     return carte, dimensions
}

class Drawable_Thing
{
	sf::RectangleShape create_rect(size_t taille_brique, sf::Color &color)
	{
		sf::RectangleShape rectangle;
		rectangle.setSize(sf::Vector2f(taille_brique, taille_brique));
		rectangle.setFillColor(color);
		rectangle.setPosition(position.x*taille_brique, position.y*taille_brique);
		return rectangle;
	}

public:
	void move(glm::ivec2 &delta, size_t taille_brique)
	{
		position += delta;
		rectangle.setPosition(position.x*taille_brique, position.y*taille_brique);
	}

	Drawable_Thing(glm::ivec2 position, size_t taille_brique, sf::Color color) :
		position(position),
		rectangle(create_rect(taille_brique, color))
	{
	}

	glm::ivec2 position;
	sf::RectangleShape rectangle;
};
class Case : public Drawable_Thing
{
	sf::Color terrain_to_color(Terrain terrain)
	{
		static const std::map<Terrain, sf::Color> table {
			{Terrain::Foret, sf::Color(0, 128, 0)},
			{Terrain::Montagne, sf::Color(128, 128, 128)},
			{Terrain::Prairie, sf::Color(255, 255, 0)},
			{Terrain::Ville, sf::Color(0, 0, 0)},
			{Terrain::Eau, sf::Color(100, 100, 255)}
		};
		return table.at(terrain);
	}

public:
	Case(glm::ivec2 position, Terrain terrain, size_t taille_brique) :
		Drawable_Thing(position, taille_brique, terrain_to_color(terrain)),
		terrain(terrain)
	{
	}

	Terrain terrain;
};

class Lynx : public Drawable_Thing
{
public:
	// Distance de déplacement en fonction de la qualité de l'habitat. 
	// H correspond à la qualité de l'habitat où se trouve l'animal à l'instant t
	double distance(double H)
	{
		// auto& gen = getRndGen();
		// std::cout << "H" << H << std::endl;
		double mean = 0;
		double sd;
		if (H < 0.33)
			sd = 3;
		else if (H < 0.66)
			sd = 2;
		else
			sd = 0.75;
		std::normal_distribution<> d {mean, sd};
		// size_t norm = (size_t)std::abs(std::round(d(gen)));// Not like in python
		// size_t norm = (size_t)std::abs(int(d(gen)));
		size_t norm = scratch_norm(std::vector<double>({mean, sd}));
		// std::cout << "norm: " << norm;

		return norm;
	}

	//Vecteurs effectués lors du déplacement. Nb sera a priori retourné par la fonction distance. Déplacement en fonction des cases adjacentes.
	glm::ivec2 direction_free(glm::ivec2 &dimensions, std::vector<double> &qualite, std::vector<std::vector<Terrain>> &carte)
	{
		std::uniform_int_distribution<> distrib(0, _max_);
		// auto& gen = getRndGen();
		auto vec = glm::ivec2(0, 0);
		// double u = (double)distrib(gen)/(double)_max_;
		double u = gener.at(gener_i);
		gener_i++;
		// std::cout << "random: direction_free " << gener_i << std::endl;
		double qterrainEst;
		if (position.x+1 > dimensions.x-1) // Est
			qterrainEst = 0;
		else
			qterrainEst = qualite.at((size_t)carte.at(position.y).at(position.x+1));
		double qterrainOuest;
		if (position.x-1 < 0) // Ouest
			qterrainOuest = 0;
		else
			qterrainOuest = qualite.at((size_t)carte.at(position.y).at(position.x-1));
		double qterrainSud;
		if (position.y+1 > dimensions.y-1) // Sud
			qterrainSud = 0;
		else
			qterrainSud = qualite.at((size_t)carte.at(position.y+1).at(position.x));
		double qterrainNord;
		if (position.y-1 < 0) // Nord
			qterrainNord = 0;
		else
			qterrainNord = qualite.at((size_t)carte.at(position.y-1).at(position.x));
		double Q = qterrainNord+qterrainSud+qterrainEst+qterrainOuest;
		if (u < qterrainEst/Q) // Est
			vec.x = 1;
		else if (u < (qterrainEst+qterrainOuest)/Q) // Ouest
			vec.x = -1;
		else if (u < (qterrainEst+qterrainOuest+qterrainSud)/Q) // Sud
			vec.y = 1;
		else // Nord
			vec.y = -1;
		return vec;
	}

	// Calcul de la distance entre le lynx et un autre individu    
	double norm_vec(glm::vec2 position1)
	{
		return std::pow(std::pow(position1.x-position.x, 2)+std::pow(position1.y-position.y, 2), 0.5);
	}

	// Vecteur de fuite du lynx lorsque la densité locale de lynx est élevée       
	glm::ivec2 direction_densite(std::vector<Lynx> &lynx, double Vision, glm::ivec2 &dimensions)
	{
		size_t N = 0;
		auto somme_position = glm::vec2(0, 0);
		for (auto &l : lynx)
			if (norm_vec(l.position) < Vision) {
				N += 1;
				somme_position += l.position;
			}
		auto vec_bary = somme_position / (float)N;
		// std::cout << "vect_bary: " << vec_bary.x << " " << vec_bary.y << std::endl;
		auto barycentre = glm::ivec2(std::round(vec_bary.x), std::round(vec_bary.y));
		// std::cout << "barycentre: " << barycentre.x << " " << barycentre.y << std::endl;
		auto vect_fuite = position - barycentre;
		// std::cout << "vect_fuite: " << vect_fuite.x << " " << vect_fuite.y << std::endl;
		if (vect_fuite.x > dimensions.x-position.x-1) // Est
			vect_fuite.x = dimensions.x-position.x-1;
		if (vect_fuite.x < 0-position.x) // Ouest
			vect_fuite.x = 0-position.x;
		if (vect_fuite.y > dimensions.y-position.y-1) // Sud
			vect_fuite.y = dimensions.y-position.y-1;
		if (vect_fuite.y < 0-position.y) // Nord
			vect_fuite.y = 0-position.y;
		return vect_fuite;
	}
		
	// Décomposition du vecteur de fuite en petits vecteurs	
	std::pair<std::vector<glm::ivec2>, std::vector<glm::ivec2>> vectorisation(glm::ivec2 &grand_vecteur)
	{
		auto vect_x = std::vector<glm::ivec2>();
		auto vect_y = std::vector<glm::ivec2>();
		for (size_t i = 0; i < std::abs(grand_vecteur.x); i++) {
			if (grand_vecteur.x < 0)
				vect_x.emplace_back(glm::ivec2(-1, 0));
			else
				vect_x.emplace_back(glm::ivec2(1, 0));
		}
		for (size_t i = 0; i < std::abs(grand_vecteur.y); i++) {
			if (grand_vecteur.y < 0)
				vect_y.emplace_back(glm::ivec2(0, -1));
			else
				vect_y.emplace_back(glm::ivec2(0, 1));
		}
		
		return std::pair<std::vector<glm::ivec2>, std::vector<glm::ivec2>>(vect_x, vect_y);
	}	
	
	// Renvoie le statut S,I,R du lynx et son état énergétique (mort ou vivant avec le niveau énergétique). Effectue la modification de position consécutive au déplacement.
	std::pair<size_t, bool> deplacement(std::vector<std::vector<Terrain>> &carte, std::vector<Lynx> &lynx, std::vector<double> &qualite, double dconst, double b, std::vector<double> &delta, double Vision, glm::ivec2 &dimensions, size_t taille_brique)
	{
		// std::cout << "pos: " << position.x << " " << position.y << std::endl;
		auto terrain = carte.at(position.y).at(position.x);
		// std::cout << "terrain: " << terrain << std::endl;
		size_t dist = distance(qualite.at((size_t)terrain));
		// std::cout << "distance: " << dist << std::endl;
		energie += -dconst+b*qualite.at((size_t)terrain);
		energie = std::min(energie, 100.0);
		for (size_t x = 0; x < dist; x++) {
			// std::cout << std::endl;
			terrain = carte.at(position.y).at(position.x);
			// std::cout << "terrain: " << terrain << std::endl;
			energie -= delta.at((size_t)terrain);
			// std::cout << "energie: " << energie << std::endl;
			auto direc = direction_free(dimensions, qualite, carte);
			move(direc, taille_brique);
			// std::cout << "deplacement: " << position.x << " " << position.y << std::endl;
		}
		auto res = direction_densite(lynx, Vision, dimensions);
		// std::cout << "directon_densite: " << res.x << " " << res.y << std::endl;
		auto vect = vectorisation(res);
		auto X = vect.first;
		auto Y = vect.second;
		for (size_t x = 0; x < X.size(); x++) {
			terrain = carte.at(position.y).at(position.x);
			energie -= delta.at((size_t)terrain);
			move(X.at(x), taille_brique);
		}
		for (size_t y = 0; y < Y.size(); y++) {
			terrain = carte.at(position.y).at(position.x);
			energie -= delta.at((size_t)terrain);
			move(Y.at(y), taille_brique);
		}
		age += 1;
		statut = class_age();
		if (energie <= 0) {
			energie = 0;
			return std::pair<size_t, bool>(statut, false);
		}
		return std::pair<size_t, bool>(statut, true);
	}
	
	// Renvoie si l'individu est juvénile, reproducteur ou stérile
	size_t class_age(void)
	{
		if (age <= 2)
			return 0;
		else if (age <= 15)
			return 1;
		else
			return 2;
	}
	
	// Initialise les caractéristiques des nouveaux individus
	Lynx(glm::ivec2 position, size_t taille_brique, sf::Color color, size_t age, double energie) :
		Drawable_Thing(position, taille_brique, color),
		age(age),
		statut(class_age()),
		energie(energie)
	{
	}

	size_t age;
	size_t statut;
	double energie;
};

bool is_in_vector(size_t value, std::vector<size_t> &vector);
int pos_in_vector(size_t value, std::vector<size_t> &vector);
double qualite_carte(glm::ivec2 &dimensions, std::vector<double> &qualite, std::vector<std::vector<size_t>> &carte, double area);
double norme(glm::vec2 &vec1, glm::vec2 &vec2);
std::vector<size_t> naissances_lynx(std::vector<Lynx> &lynx, double K, double oscillation, double ksi);
size_t nombre_lynx_categorie(std::vector<Lynx> &lynx, size_t cat);
size_t nombre_lynx_age(std::vector<Lynx> &lynx, size_t age);
size_t binomiale(size_t n, double p);
void mortalite_risque(std::vector<Lynx> &lynx, std::vector<std::vector<bool>> &carte_routiere, double traffic, std::vector<double> &Taux_de_mortalites, std::vector<double> &Vulnerabilite, double chasse, size_t &nombre_morts_S, size_t &nombre_morts_I, size_t &nombre_morts_R, size_t &lynx_morts_route, size_t &nombre_morts_chasse);
