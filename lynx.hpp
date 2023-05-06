#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <glm/vec2.hpp>
#include <cmath>
#include <random>

static inline constexpr size_t _max_ = 123456789;

static size_t seed = 0;

auto& getRndGen() {
	//std::cout << "ACCESS RND GEN: seed is " << seed << std::endl;
	static std::mt19937 gen(seed);
	return gen;
}

class Lynx
{
public:
	// Distance de déplacement en fonction de la qualité de l'habitat. 
	// H correspond à la qualité de l'habitat où se trouve l'animal à l'instant t
	double distance(double H)
	{
		auto& gen = getRndGen();
		// std::cout << "H" << H << std::endl;
		double moyenne = 0;
		double ecart_type;
		if (H < 0.33)
			ecart_type = 3;
		else if (H < 0.66)
			ecart_type = 2;
		else
			ecart_type = 0.75;
		std::normal_distribution<> d {moyenne, ecart_type};
		size_t norm = (size_t)std::abs(std::round(d(gen)));
		// std::cout << "norm: " << norm;

		return norm;
	}

	//Vecteurs effectués lors du déplacement. Nb sera a priori retourné par la fonction distance. Déplacement en fonction des cases adjacentes.
	glm::ivec2 direction_free(glm::ivec2 &dimensions, std::vector<double> &qualite, std::vector<std::vector<size_t>> &carte)
	{
		std::uniform_int_distribution<> distrib(0, _max_);
		auto& gen = getRndGen();
		auto vec = glm::ivec2(0, 0);
		double u = (double)distrib(gen)/(double)_max_;
		double qterrainEst;
		if (position.x+1 == dimensions.x) // Est
			qterrainEst = 0;
		else
			qterrainEst = qualite.at(carte.at(position.y).at(position.x+1));
		double qterrainOuest;
		if (position.x-1 == 0) // Ouest
			qterrainOuest = 0;
		else
			qterrainOuest = qualite.at(carte.at(position.y).at(position.x-1));
		double qterrainSud;
		if (position.y+1 == dimensions.y) // Sud
			qterrainSud = 0;
		else
			qterrainSud = qualite.at(carte.at(position.y+1).at(position.x));
		double qterrainNord;
		if (position.y-1 == 0) // Nord
			qterrainNord = 0;
		else
			qterrainNord = qualite.at(carte.at(position.y-1).at(position.x));
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
		auto somme_position = glm::ivec2(0, 0);
		for (auto &l : lynx)
			if (norm_vec(l.position) < Vision) {
				N += 1;
				somme_position += l.position;
			}
		auto vec_bary = somme_position / (int)N;
		auto barycentre = glm::ivec2(std::round(vec_bary.x), std::round(vec_bary.y));
		auto vect_fuite = position - barycentre;
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
	std::pair<size_t, bool> deplacement(std::vector<std::vector<size_t>> &carte, std::vector<Lynx> &lynx, std::vector<double> &qualite, double dconst, double b, std::vector<double> &delta, double Vision, glm::ivec2 &dimensions)
	{
		std::cout << "pos: " << position.x << " " << position.y << std::endl;
		size_t terrain = carte.at(position.y).at(position.x);
		// std::cout << "terrain: " << terrain << std::endl;
		size_t dist = distance(qualite.at(terrain));
		energie += -dconst+b*qualite.at(terrain);
		energie = std::min(energie, 100.0);
		for (size_t x = 0; x < dist; x++) {
			// std::cout << std::endl;
			terrain = carte.at(position.y).at(position.x);
			// std::cout << "terrain: " << terrain << std::endl;
			energie -= delta.at(terrain);
			// std::cout << "energie: " << energie << std::endl;
			auto direc = direction_free(dimensions, qualite, carte);
			position += direc;
			// std::cout << "deplacement: " << x << position << std::endl;
		}
		// std::cout << "directon_densite: " << direction_densite(lynx) << std::endl;
		auto res = direction_densite(lynx, Vision, dimensions);
		auto vect = vectorisation(res);
		auto X = vect.first;
		auto Y = vect.second;
		for (size_t x = 0; x < X.size(); x++) {
			terrain = carte.at(position.y).at(position.x);
			energie -= delta.at(terrain);
			position += X[x];
		}
		for (size_t y = 0; y < Y.size(); y++) {
			terrain = carte.at(position.y).at(position.x);
			energie -= delta.at(terrain);
			position += Y[y];  
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
	Lynx(glm::ivec2 position, size_t age, double energie) :
		position(position),
		age(age),
		statut(class_age()),
		energie(energie)
	{
	}

	glm::ivec2 position;
	size_t age;
	size_t statut;
	double energie;
};

bool is_in_vector(size_t value, std::vector<size_t> &vector);
double qualite_carte(glm::ivec2 &dimensions, std::vector<double> &qualite, std::vector<std::vector<size_t>> &carte, double area);
double norme(glm::vec2 &vec1, glm::vec2 &vec2);
std::vector<size_t> naissances_lynx(std::vector<Lynx> &lynx, double K, double oscillation, double ksi);
size_t nombre_lynx_categorie(std::vector<Lynx> &lynx, size_t cat);
size_t nombre_lynx_age(std::vector<Lynx> &lynx, size_t age);
size_t binomiale(size_t n, double p);
void mortalite_risque(std::vector<Lynx> &lynx, std::vector<std::vector<bool>> &carte_routiere, double traffic, std::vector<double> &Taux_de_mortalites, std::vector<double> &Vulnerabilite, double chasse, size_t &nombre_morts_S, size_t &nombre_morts_I, size_t &nombre_morts_R, size_t &lynx_morts_route, size_t &nombre_morts_chasse);
