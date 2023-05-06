#include "lynx.hpp"

bool is_in_vector(size_t value, std::vector<size_t> &vector)
{
	for (auto &v : vector) {
		if (value == v)
			return true;
	}
	return false;
}

double qualite_carte(glm::ivec2 &dimensions, std::vector<double> &qualite, std::vector<std::vector<size_t>> &carte, double area)
{
	double q = 0;
	for (int j = 0; j < dimensions.y; j++)
		for (int i = 0; i < dimensions.x; i++)
			q += qualite.at(carte.at(j).at(i));
	return q / area;
}

double norme(glm::vec2 &vec1, glm::vec2 &vec2)
{
	return std::pow(std::pow(vec1.x-vec2.x, 2)+std::pow(vec1.y-vec2.y, 2), 0.5);
}

// Calcule le nombre de lynx qui naissent cette année, et renvoie les indices de leurs parents
std::vector<size_t> naissances_lynx(std::vector<Lynx> &lynx, double K, double oscillation, double ksi)
{
	std::sort(lynx.begin(), lynx.end(), [](Lynx lynx1, Lynx lynx2){return lynx1.statut < lynx2.statut;});
	size_t nbS = nombre_lynx_categorie(lynx, 0);
	size_t nbI = nombre_lynx_categorie(lynx, 1);
	size_t nombre_naissances = binomiale(nbI, (std::pow(1-lynx.size()/K, oscillation)) * ksi);
	// nombre_naissances = max(round(nombre_naissances),0);
	// std::cout << "nombre_naissances: " << nombre_naissances << std::endl;
	auto indices_naissances = std::vector<size_t>();
	auto indices_I = std::vector<size_t>();
	for (size_t k = nbS; k < nbS+nbI; k++) {
		indices_I.emplace_back(k);
	}
	std::random_shuffle(indices_I.begin(), indices_I.end());
	for (size_t n = 0; n < nombre_naissances; n++) {
		indices_naissances.emplace_back(indices_I.at(n));
	}
	return indices_naissances;
}

// Calcule le nombre de lynx de chaque catégorie d'âge dans une population donnée
size_t nombre_lynx_categorie(std::vector<Lynx> &lynx, size_t cat)
{
	size_t nombre_lynx = 0;
	for (auto &l : lynx)
		if (l.statut == cat)
			nombre_lynx+=1;
	return nombre_lynx;
}

// Calcule le nombre de lynx pour un âge donné
size_t nombre_lynx_age(std::vector<Lynx> &lynx, size_t age)
{
	size_t nombre_lynx = 0;
	for (auto &l : lynx)
		if (l.age == age)
			nombre_lynx+=1;
	return nombre_lynx;
}

// Modélisation d'une variable X suivant une loi binomiale de paramètres (n,p)
size_t binomiale(size_t n, double p)
{
	auto& gen = getRndGen();
	std::uniform_int_distribution<> distrib(0, _max_);
	size_t X = 0;
	for (size_t i = 0; i < n; i++) {
		double u = (double)distrib(gen)/(double)_max_;
		if (u < p)
			X += 1;
	}
	return X;
}

// Calcule le nombre de lynx allant mourir dans chaque catégorie d'âge et renvoie la nouvelle population (survivants)
void mortalite_risque(std::vector<Lynx> &lynx, std::vector<std::vector<bool>> &carte_routiere, double traffic, std::vector<double> &Taux_de_mortalites, std::vector<double> &Vulnerabilite, double chasse, size_t &nombre_morts_S, size_t &nombre_morts_I, size_t &nombre_morts_R, size_t &lynx_morts_route, size_t &nombre_morts_chasse)
{
	auto& gen = getRndGen();
	std::uniform_int_distribution<> distrib(0, _max_);
	std::cout << "lynx" << lynx.size() << std::endl;
	lynx_morts_route = 0;
	for (size_t l = 0; l < lynx.size(); l++) {
		size_t d = l-lynx_morts_route;
		if (carte_routiere.at(lynx.at(d).position.y).at(lynx.at(d).position.x) == true) {
			double u = (double)distrib(gen)/(double)_max_;
			if (u < traffic) {
				lynx.erase(lynx.begin()+d);
				std::cout << "RIP lynx sur le côté de la route" << std::endl;
				lynx_morts_route += 1;
			}
		}
	}
	std::sort(lynx.begin(), lynx.end(), [](Lynx lynx1, Lynx lynx2){return lynx1.statut < lynx2.statut;});
	size_t nbS = nombre_lynx_categorie(lynx, 0);
	size_t nbI = nombre_lynx_categorie(lynx, 1);
	size_t nbR = nombre_lynx_categorie(lynx, 2);
	nombre_morts_S = binomiale(nbS, Taux_de_mortalites.at(0)*0.5);
	nombre_morts_I = binomiale(nbI, Taux_de_mortalites.at(1)/13);
	nombre_morts_R = binomiale(nbR, Taux_de_mortalites.at(2));
	// std::cout << nombre_morts_S << " " << nombre_morts_I << " " << nombre_morts_R << std::endl;
	size_t nombre_morts_nat = nombre_morts_S+nombre_morts_I+nombre_morts_R;
	nombre_morts_S += binomiale(nbS-nombre_morts_S, Vulnerabilite.at(0)*chasse);
	nombre_morts_I += binomiale(nbI-nombre_morts_I, Vulnerabilite.at(1)*chasse);
	nombre_morts_R += binomiale(nbR-nombre_morts_R, Vulnerabilite.at(2)*chasse);
	size_t nombre_morts_tot = nombre_morts_S+nombre_morts_I+nombre_morts_R;
	nombre_morts_chasse = nombre_morts_tot-nombre_morts_nat;
	// std::cout << nombre_morts_S << nombre_morts_I << nombre_morts_R;
	auto indices_morts = std::vector<size_t>();
	auto indices_S = std::vector<size_t>();
	for (size_t k = 0; k < nbS; k++) {
		indices_S.emplace_back(k);
	}
	std::random_shuffle(indices_S.begin(), indices_S.end());
	auto indices_I = std::vector<size_t>();
	for (size_t k = nbS; k < nbS+nbI; k++) {
		indices_I.emplace_back(k);
	}
	std::random_shuffle(indices_I.begin(), indices_I.end());
	auto indices_R = std::vector<size_t>();
	for (size_t k = 0; k < nbS+nbI+nbR; k++) {
		indices_R.emplace_back(k);
	}
	std::random_shuffle(indices_R.begin(), indices_R.end());
	for (size_t n = 0 ; n < nombre_morts_S; n++) {
		indices_morts.emplace_back(indices_S.at(n));
	}
	for (size_t n = 0 ; n < nombre_morts_I; n++) {
		indices_morts.emplace_back(indices_I.at(n));
	}
	for (size_t n = 0 ; n < nombre_morts_R; n++) {
		indices_morts.emplace_back(indices_R.at(n));
	}
	for (int k = lynx.size()-1; k >= 0; k--) {
		if (is_in_vector(k, indices_morts)) {
			lynx.erase(lynx.begin()+k);
			std::cout << "Lynx mort naturelle" << std::endl;
		}
	}
	while (true) {
		a = 0;
	}
}

int main(void)
// int main(int argc, char **argv)
{
	auto& gen = getRndGen();
	std::uniform_int_distribution<> distrib(0, _max_);

	// Coefficient de perte d'énergie lié au déplacement d'une case dans un habitat
	auto delta = std::vector<double>({2, 4, 6, 8, 10});
	// Coefficient de recouvrement d'énergie lié à la qualité de vie de l'habitat
	auto qualite = std::vector<double>({1, 0.7, 0.5, 0.2, 0.001});
	// Perte d'énergie physiologique
	double dconst = 10;
	// Taux de recouvrement d'énergie optimal
	double b = 50;
	// Durée moyenne à l'état S (temps avant maturité sexuelle)
	// double alpha = 0.5;
	// Durée moyenne à l'état I
	// double beta = 1/15;
	// Taux de mortalité 
	auto Taux_de_mortalites = std::vector<double>({0.45, 0.2, 0.35});
	// Vulnérabilité aux chasseurs
	auto Vulnerabilite = std::vector<double>({0.225, 0.8, 1});
	// Amortissement du nombre de naissances
	double oscillation = 1;
	// Pression de chasse 
	double braconnage = 1;
	double chasse = 0.25*braconnage;
	// Nombre de petits par saison
	double ksi = 2;
	// Taux de mortalité supplémentaire lié au traffic routier
	double traffic = 0.125;
	// Nombre de simulations
	size_t NSimul = 1;
	// Nombre d'années de simulation
	size_t Years = 20;
	// Compartiments de population
	auto Compartiments = std::vector<std::string>({"S", "I", "R"});
	// Distance de territoire
	double Vision = 5;
	// Nombre de km par pixel de la carte de France téléchargée
	// size_t km_par_px = 1;

	auto dimensions = glm::ivec2(16, 16);
	auto carte = std::vector<std::vector<size_t>>();
	for (int i = 0; i < dimensions.y; i++) {
		auto row = std::vector<size_t>();
		for (int j = 0; j < dimensions.x; j++) {
			row.emplace_back(0);
		}
		carte.emplace_back(row);
	}
	size_t area = dimensions.y * dimensions.x;

	// double Qmoyenne263 = 0.35*qualite.at(0) + 0.6*qualite.at(2) + 0.04*qualite.at(3) + 0.01*qualite.at(4);
	double Qmoyenne263 = 0.65801;
	double Qcarte = qualite_carte(dimensions, qualite, carte, area);
	double K = Qcarte / Qmoyenne263 * 2.63;
	std::cout << "Qcarte: " << Qcarte << std::endl;
	std::cout << "K: " << K << std::endl;
	K *= area/100;

	auto carte_routiere = std::vector<std::vector<bool>>();
	for (int j = 0; j < dimensions.y; j++) {
		auto ligne = std::vector<bool>();
		for (int i = 0; i < dimensions.x; i++) {
			ligne.emplace_back(false);
		}
		carte_routiere.emplace_back(ligne);
	}

	// indices = [0, 	         1,	         2,		    3,	     4]
	auto habitats = std::vector<std::string>({"forêt", "montagne", "prairie", "ville", "eau"});
	// auto couleurs_terrain = std::vector<sf::Color>({sf::Color(0, 128, 0), sf::Color(128, 128, 128), sf::Color(255, 255, 0), sf::Color(0, 0, 0), sf::Color(100, 100, 255)});
	// auto couleurs_lynx = std::vector<sf::Color>({sf::Color(0, 0, 0), sf::Color(255, 0, 0), sf::Color(255, 165, 0)});
	// for (size_t i = 0; i < 50; i++)
	// 	couleurs_lynx.append(sf::Color(255, 255, 255));

	// pg.display.init()
	// objet_info = pg.display.Info()
	// f = 0.9
	// taille_ecran = (int(objet_info.current_w),int(objet_info.current_h))
	// if (dimensions.x/objet_info.current_w > dimensions.y/objet_info.current_h) { // axe x limitant
	// 	width = int(objet_info.current_w*f)
	// 	taille_brique = width/dimensions[0]
	// 	height = taille_brique*dimensions[1]
	// 	print("OUI")
	// } else {
	// 	height = int(objet_info.current_h*f)
	// 	taille_brique = height/dimensions[1]
	// 	width = taille_brique*dimensions[0]
	// 	print("NON")
	// }
	// print("w, h", width, height)
	// taille_ecran = (width, height)
	// print("tb", taille_brique)
	// print("w*tb", width/taille_brique)
	// print("len(carte), taille_ecran", len(carte), taille_ecran)
	// ecran = pg.display.set_mode(taille_ecran)
	// # ecran = pg.display.set_mode(taille_ecran, pg.FULLSCREEN)

	auto carte_densite = std::vector<std::vector<size_t>>();
	for (int j = 0; j < dimensions.y; j++) {
		auto ligne = std::vector<size_t>();
		for (int i = 0; i < dimensions.x; i++) {
			ligne.emplace_back(0);
		}
		carte_densite.emplace_back(ligne);
	}

	// Boucle principale
	size_t Nb_Succes_25 = 0;
	size_t Nb_Succes_50 = 0;
	size_t Nb_Succes_75 = 0;
	size_t Nb_Succes_100 = 0;
	size_t max_year = 0;
	for (size_t g = 0; g < NSimul; g++) {
		std::cout << "Simulation " << g << std::endl;
		auto lynx = std::vector<Lynx>();
		// Pour lâcher aléatoirement (position et âge) un nombre voulu de lynx 
		size_t nb_lynx = 5;
		for (size_t i = 0; i < nb_lynx; i++) {
			size_t x = distrib(gen)%dimensions.x;
			size_t y = distrib(gen)%dimensions.y;
			size_t age = distrib(gen)%21;
			double energie = 100;
			lynx.emplace_back(Lynx(glm::ivec2(x, y), age, energie));
		}
		auto Mortalites = std::vector<std::vector<size_t>>();
		auto Survivants = std::vector<std::vector<size_t>>();
		for (size_t i = 0; i < Compartiments.size(); i++) {
			auto years = std::vector<size_t>();
			auto years2 = std::vector<size_t>();
			for (size_t k = 0; k < Years; k++) {
				years.emplace_back(0);
				years2.emplace_back(0);
			}
			Mortalites.emplace_back(years);
			Survivants.emplace_back(years2);
		}
		auto Mortalites_energetique = std::vector<size_t>();
		auto Mortalites_routiere = std::vector<size_t>();
		auto Mortalites_chasse = std::vector<size_t>();
		for (size_t k = 0; k < Years; k++) {
			Mortalites_energetique.emplace_back(0);
			Mortalites_routiere.emplace_back(0);
			Mortalites_chasse.emplace_back(0);
		}
		for (size_t year = 0; year < Years; year++) {
			std::cout << std::endl << "Annee " << year << std::endl;
			size_t nombre_morts_S;
			size_t nombre_morts_I;
			size_t nombre_morts_R;
			size_t lynx_morts_route;
			size_t nombre_morts_chasse;
			mortalite_risque(lynx, carte_routiere, traffic, Taux_de_mortalites, Vulnerabilite, chasse, nombre_morts_S, nombre_morts_I, nombre_morts_R, lynx_morts_route, nombre_morts_chasse);
			Mortalites.at(0).at(year) += nombre_morts_S;
			Mortalites.at(1).at(year) += nombre_morts_I;
			Mortalites.at(2).at(year) += nombre_morts_R;
			Mortalites_routiere.at(year) += lynx_morts_route;
			Mortalites_chasse.at(year) += nombre_morts_chasse;
			auto indices_parents = naissances_lynx(lynx, K, oscillation, ksi);
			size_t lynx_morts = 0;
			// std::cout << "len: " << lynx.size()+indices_parents.size() << std::endl;
			for (size_t l = 0; l < lynx.size()+indices_parents.size(); l++) {
				size_t d = l-lynx_morts;
				// std::cout << "d: " << d;
				// std::cout << "lynx_morts: " << lynx_morts;
				auto result = lynx.at(d).deplacement(carte, lynx, qualite, dconst, b, delta, Vision, dimensions);
				size_t statut = result.first;
				size_t vivant = result.second;
				while (is_in_vector(l, indices_parents)) {
					// std::cout << "indices_parents: " << indices_parents << std::endl;
					size_t x = lynx.at(d).position.x;
					size_t y = lynx.at(d).position.y;
					size_t age = 0;
					double energie = 100;
					lynx.emplace_back(Lynx(glm::ivec2(x, y), age, energie));
					indices_parents.erase(indices_parents.begin()+l);
				}
				if (vivant == false) {
					Mortalites.at(statut).at(year) += 1;
					Mortalites_energetique.at(year) += 1;
					lynx.erase(lynx.begin()+d);
					std::cout << "Lynx mort energie" << std::endl;
					lynx_morts += 1;
				} else {
					Survivants.at(statut).at(year) += 1;
				}
			}
			// std::cout << "Survivants S, I, R :" << Survivants.at(0).at(year) << " " << Survivants.at(1).at(year) << " " << Survivants.at(2).at(year) << std::endl;
			// fig, ax = plt.subplots()
			// ax.set_xlim(0, dimensions[0])
			// ax.set_ylim(dimensions[1], 0)
			// plt.grid()
			auto lynx_couleur = std::vector<std::vector<size_t>>();
			for (int j = 0; j < dimensions.y; j++) {
				auto ligne = std::vector<size_t>();
				for (int i = 0; i < dimensions.x; i++) {
					ligne.emplace_back(0);
				}
				lynx_couleur.emplace_back(ligne);
			}
			for (auto &l : lynx) {
				lynx_couleur.at(l.position.y).at(l.position.x) += 1;
				carte_densite.at(l.position.y).at(l.position.x) += 1;
			}
			// std::cout << lynx_couleur;
			// for i in range(dimensions[0]):
			// 	for j in range(dimensions[1]):
			// 	couleur = couleurs_terrain[int(carte[j, i])]
			// 	ax.add_patch(Rectangle((i, j), 1, 1, color=couleur))
			// 	if lynx_couleur[j, i] != 0:
			// 		# print(int(lynx_couleur[j, i]), couleurs_lynx)
			// 		couleur = couleurs_lynx[int(lynx_couleur[j, i])]
			// 		ax.add_patch(Rectangle((i+0.25, j+0.25), 0.5, 0.5, color=couleur))
			// 	if carte_routiere[j, i] == 1:
			// 		ax.add_patch(Rectangle((i, j), 1, 1, color="black", alpha=0.5))
			// plt.show()
			// affichage_ecran(ecran, dimensions, taille_brique, couleurs_terrain, couleurs_lynx, carte, carte_routiere, lynx_couleur)
			// t.sleep(0.1)
			// if terminer_programme(pg.K_LCTRL):
			// break
			max_year = year;
		}
		if (Survivants.at(1).at(int(24*Years/100))+Survivants.at(0).at(int(24*Years/100)) > 0)
			Nb_Succes_25 += 1;
		if (Survivants.at(1).at(int(49*Years/100))+Survivants.at(0).at(int(49*Years/100)) > 0)
			Nb_Succes_50 += 1;
		if (Survivants.at(1).at(int(74*Years/100))+Survivants.at(0).at(int(74*Years/100)) > 0)
			Nb_Succes_75 += 1;
		if (Survivants.at(1).at(int(99*Years/100))+Survivants.at(0).at(int(99*Years/100)) > 0)
			Nb_Succes_100 += 1;
		std::cout << Nb_Succes_25 << " " << Nb_Succes_50 << " " << Nb_Succes_75 << " " << Nb_Succes_100 << std::endl;
	}
	for (size_t j = 0; j < carte_densite.size(); j++)
		for (size_t i = 0; i < carte_densite.at(j).size(); i++)
			carte_densite.at(j).at(i) *= 20/max_year;
	// print(carte_densite)
	// while True
	// affichage_densite(ecran, dimensions, taille_brique, couleurs_terrain, carte, carte_routiere, carte_densite)
	// t.sleep(0.01)
	// if terminer_programme(pg.K_LSHIFT):
	// 	break
	// pg.display.quit()

	return 0;
}