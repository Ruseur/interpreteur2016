#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}
int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::traduitEnCPP(ostream & cout, unsigned int indentation) const {
  for (int i = 0; i < m_instructions.size(); i++) {
    m_instructions[i]->traduitEnCPP(cout, indentation); // on exécute chaque instruction de la séquence
		
		if(typeid(*m_instructions[i]) != typeid(NoeudInstTantQue) && typeid(*m_instructions[i]) != typeid(NoeudInstSi) && typeid(*m_instructions[i]) != typeid(NoeudInstPour) && typeid(*m_instructions[i]) != typeid(NoeudInstSwitch) ) {
			cout << ";";
		}
		cout <<endl;
	}
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	m_variable->traduitEnCPP(cout, indentation);
	cout << " = ";
	m_expression->traduitEnCPP(cout,0);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	if(m_operandeGauche != nullptr) {
		m_operandeGauche->traduitEnCPP(cout, 0);
	}
	if(m_operateur != "") {
		cout << m_operateur.getChaine();
	}
	if(m_operandeDroit != nullptr) {
		m_operandeDroit->traduitEnCPP(cout,0);
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(vector<Noeud*> conditions, vector<Noeud*> sequences)
: m_condition(conditions), m_sequence(sequences) {
}

int NoeudInstSi::executer() {
	
	int i=0; // indice pour déterminer quel séquence executer
	bool termine = false; //afin de sortir de la loop en cas de conditions TRUE
	
	// On loop tant qu'il reste des conditions non TRUE
	while(termine == false && i< m_condition.size()) {
		if(m_condition[i]->executer()) { //On teste la condition de l'indice i
			termine = true;
			i = i-1; //Je réduis l'indice de 1 si la condition est bonne afin d'executer la séquence associée à ce sinon si
		}
		i++;
	}
	if(i<m_sequence.size()) { // Je vérifie si, même si aucune condition n'est respectée, il reste une séquence qui est le "sinon"
		m_sequence[i]->executer();
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout, unsigned int indentation) const {

	cout << setw(indentation) <<""<< "if(";
	m_condition[0]->traduitEnCPP(cout,0);
	cout << setw(indentation) << ") {" << endl;
	
	m_sequence[0]->traduitEnCPP(cout,indentation+2);
	
	cout << setw(indentation) <<""<< "}";

	int i=1;
	while(i< (m_condition.size()-1)) {
		cout << setw(indentation) <<""<< "else if(";
		m_condition[i]->traduitEnCPP(cout,0);
		cout << ") {" << endl;
		m_sequence[i]->traduitEnCPP(cout, indentation+2);
		cout << setw(indentation) << "}";
		i++;
	}
	if(i < m_sequence.size()) {
		cout << "else {" << endl;
		m_sequence[i]->traduitEnCPP(cout,indentation+2);
		cout << setw(indentation)<<""<< "}";
	}
	cout << endl;
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
	while(m_condition->executer()) {
		m_sequence->executer();
	}
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	cout << setw(indentation) << "" << "while(";
	m_condition->traduitEnCPP(cout,0);
	cout << ") {" << endl;
	m_sequence->traduitEnCPP(cout, 2+indentation);
	cout << setw(indentation) << "" << "}";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {
	do {
		m_sequence->executer();
		cout << "coucou" << endl;
	}
	while(m_condition->executer());
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstRepeter::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	cout << setw(indentation) << "" << "do{" << endl;
	m_sequence->traduitEnCPP(cout, 2+indentation);
	cout <<setw(indentation) <<""<< "}" << "while (";
	m_condition->traduitEnCPP(cout,0);
	cout << ")";
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* affectation1, Noeud* condition, Noeud* affectation2, Noeud* sequence)
: m_affectation1(affectation1), m_condition(condition),m_affectation2(affectation2), m_sequence(sequence) {
}

int NoeudInstPour::executer() {
	
	if(m_affectation1 == nullptr && m_affectation2 == nullptr) {
		for( ; m_condition->executer(); ) {
			m_sequence->executer();
		}
	}
	else if(m_affectation1 == nullptr) {
		for( ; m_condition->executer(); m_affectation2->executer() ) {
			m_sequence->executer();
		}
	}
	else if(m_affectation2 == nullptr) {
		for(m_affectation1->executer(); m_condition->executer(); ) {
			m_sequence->executer();
		}
	}
	else {
		for(m_affectation1->executer(); m_condition->executer(); m_affectation2->executer()) {
			m_sequence->executer();
		}
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstPour::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
	cout << setw(indentation) << "" << "for(";
	if(m_affectation1 != nullptr) {
		m_affectation1->traduitEnCPP(cout,0);
	}
	
	cout << ";";
	m_condition->traduitEnCPP(cout,0);
	cout << ";";
	
	if(m_affectation2 != nullptr) {
		m_affectation2->traduitEnCPP(cout,0);
	}
	
	cout <<") {" << endl;
	m_sequence->traduitEnCPP(cout,indentation+2);
	cout << setw(indentation) << "" <<  "}";
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() {
}

int NoeudInstEcrire::executer() {
	
	for(Noeud* temp : m_expressions) {
		if( (typeid(*temp) == typeid(SymboleValue) && *((SymboleValue*)temp)== "<CHAINE>")) {
			string string_temp = ((SymboleValue*)temp)->getChaine();
			string_temp.erase(0,1);
			string_temp.erase(string_temp.size()-1,1);
			cout << string_temp;
		}else {
			cout << temp->executer();
		}
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstEcrire::ajoute(Noeud* expression) {
  if (expression!=nullptr) m_expressions.push_back(expression);
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
	cout << setw(indentation) << "" << "cout ";
	for(Noeud* temp : m_expressions) {
		cout << "<<";
		temp->traduitEnCPP(cout,0);
	}
}


////////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire() {
}

int NoeudInstLire::executer() {
	for(Noeud* temp : m_variables) {
		int temp2;
		cout << "Entrez la valeur de la variable " << ((SymboleValue*)temp)->getChaine() << " : ";
		cin >> temp2;

		((SymboleValue*) temp)->setValeur(temp2);
	}
	
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstLire::ajoute(Noeud* variable) {
  if (variable!=nullptr) m_variables.push_back(variable);
}
void NoeudInstLire::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
	cout << setw(indentation) << "" << "cin ";
	for(Noeud* temp : m_variables) {
		cout << ">>";
		temp->traduitEnCPP(cout,0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSwitch
////////////////////////////////////////////////////////////////////////////////

NoeudInstSwitch::NoeudInstSwitch(vector<Noeud*> ent, vector<Noeud*> seq, Noeud* var) :
m_entiers(ent),
m_sequence(seq),
m_variable(var) {
}


int NoeudInstSwitch::executer() {
	bool passe = false;
	unsigned int i = 0;

	while (i < m_sequence.size() and !passe) {
		if (i < m_entiers.size() and m_entiers[i]->executer() == m_variable->executer()) {
			m_sequence[i]->executer();
			passe = true;
		} else if (m_sequence.size() > m_entiers.size() and i >= m_entiers.size())
			m_sequence[i]->executer();
		i++;
	}

	return 0;
}


void NoeudInstSwitch::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	
		cout << setw(indentation) <<""<< "switch(";
		m_variable->traduitEnCPP(cout,0);
		cout << setw(indentation) << ") {" << endl;

		int i=0;
		while(i< (m_entiers.size())) {
			cout << setw(indentation) <<""<< "case ";
			m_entiers[i]->traduitEnCPP(cout,0);
			cout << " :" << endl;
			m_sequence[i]->traduitEnCPP(cout, indentation+2);
			cout << endl;
			i++;
		}
	
		cout << setw(indentation) <<""<< "}";
	
}



