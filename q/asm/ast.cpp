#include "asm/ast.hpp"

#include <cassert>
#include <string>
#include <format>

#include "asm/ast.hpp"
#include "vm/cpu.hpp"

char * Ast_Namen(Asm::Deklaration::Art art);
char * Ast_Namen(Asm::Ausdruck::Art art);
char * Ast_Namen(Asm::Anweisung::Art art);

void Einschub_Ausgeben(uint8_t tiefe, std::ostream &ausgabe);
uint32_t Register_Id(const char * name);

namespace Asm {

// modul {{{
Modul::Modul(Spanne spanne, std::string name, uint16_t adresse)
    : _spanne(spanne)
    , _name(name)
    , _adresse(adresse)
{
}

Spanne
Modul::spanne() const
{
    return _spanne;
}

std::string
Modul::name() const
{
    return _name;
}

uint16_t
Modul::adresse() const
{
    return _adresse;
}

std::list<Deklaration *>
Modul::deklarationen()
{
    return _deklarationen;
}

void
Modul::deklaration_hinzufügen(Deklaration *deklaration)
{
    _deklarationen.push_back(deklaration);
}

std::list<Anweisung *>
Modul::anweisungen()
{
    return _anweisungen;
}

void
Modul::anweisung_hinzufügen(Anweisung *anweisung)
{
    _anweisungen.push_back(anweisung);
}
// }}}
// deklaration {{{
Deklaration::Deklaration(Deklaration::Art art, Spanne spanne, std::string name, bool exportieren)
    : _art(art)
    , _spanne(spanne)
    , _name(name)
    , _exportieren(exportieren)
{
}

Deklaration::Art
Deklaration::art() const
{
    return _art;
}

Spanne
Deklaration::spanne() const
{
    return _spanne;
}

std::string
Deklaration::name()
{
    return _name;
}

bool
Deklaration::exportieren()
{
    return _exportieren;
}

template<typename T>
T Deklaration::als()
{
    return static_cast<T> (this);
}

Deklaration_Konstante::Deklaration_Konstante(Spanne spanne, std::string name, uint16_t wert, bool exportieren)
    : Deklaration(Deklaration::KONSTANTE, spanne, name, exportieren)
    , _wert(wert)
{
}

void
Deklaration_Konstante::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    /*_name->ausgeben(tiefe+1);*/
    /*_wert->ausgeben(tiefe+1);*/
}

Deklaration_Daten::Deklaration_Daten(Spanne spanne, uint16_t größe, std::string name,
                                     uint16_t anzahl, std::vector<Ausdruck_Hex *> daten, bool exportieren)
    : Deklaration(Deklaration::DATEN, spanne, name, exportieren)
    , _größe(größe)
    , _anzahl(anzahl)
    , _daten(daten)
{
}

void
Deklaration_Daten::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;

    for (auto d : _daten)
    {
        d->ausgeben(tiefe+1, ausgabe);
    }

    ausgabe << std::endl;
}

uint16_t
Deklaration_Daten::anzahl()
{
    return _anzahl;
}

uint16_t
Deklaration_Daten::größe()
{
    return _größe;
}

uint32_t
Deklaration_Daten::gesamtgröße()
{
    uint32_t erg = _größe * _anzahl;

    return erg;
}

std::vector<Ausdruck_Hex *>
Deklaration_Daten::daten()
{
    return _daten;
}

Deklaration_Schablone::Deklaration_Schablone(Spanne spanne, std::string name,
                                             std::vector<Deklaration_Schablone::Feld *> felder)
    : Deklaration(Deklaration::SCHABLONE, spanne, name)
    , _felder(felder)
{
}

void
Deklaration_Schablone::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    ausgabe << "Schablone" << std::endl;
}

std::vector<Deklaration_Schablone::Feld *>
Deklaration_Schablone::felder()
{
    return _felder;
}

Deklaration_Makro::Deklaration_Makro(Spanne spanne, std::string name,
        std::vector<Ausdruck_Name *> parameter, std::vector<Anweisung *> rumpf)
    : Deklaration(Deklaration::MAKRO, spanne, name)
    , _parameter(parameter)
    , _rumpf(rumpf)
{
}

std::vector<Ausdruck_Name *>
Deklaration_Makro::parameter()
{
    return _parameter;
}

std::vector<Anweisung *>
Deklaration_Makro::rumpf()
{
    return _rumpf;
}

void
Deklaration_Makro::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << name() << std::endl;

    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << "Parameter" << std::endl;
    for (auto *param : _parameter)
    {
        param->ausgeben(tiefe + 1, ausgabe);
    }

    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << "Rumpf" << std::endl;

    for (auto *anweisung : _rumpf)
    {
        anweisung->ausgeben(tiefe + 1, ausgabe);
    }
}
// }}}
// ausdruck {{{
Ausdruck::Art
Ausdruck::art() const
{
    return _art;
}

Spanne
Ausdruck::spanne() const
{
    return _spanne;
}

bool
Ausdruck::ungleich(Art art)
{
    auto erg = _art != art;

    return erg;
}

bool
Ausdruck::gleich(Art art)
{
    auto erg = _art == art;

    return erg;
}

template<typename T>
T Ausdruck::als()
{
    return static_cast<T> (this);
}

Ausdruck_Als::Ausdruck_Als(Spanne spanne, Ausdruck *schablone, Ausdruck_Feld *variable)
    : Ausdruck(Ausdruck::ALS, spanne)
    , _schablone(schablone)
    , _variable(variable)
{
}

void
Ausdruck_Als::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _schablone->ausgeben(tiefe+1, ausgabe);
    _variable->ausgeben(tiefe+1, ausgabe);
}

Ausdruck *
Ausdruck_Als::kopie()
{
    Ausdruck_Als *erg = new Ausdruck_Als(spanne(), _schablone, _variable);

    return erg;
}

Ausdruck *
Ausdruck_Als::schablone()
{
    return _schablone;
}

Ausdruck_Feld *
Ausdruck_Als::variable()
{
    return _variable;
}

Ausdruck_Bin::Ausdruck_Bin(Spanne spanne, Token *op, Ausdruck *links, Ausdruck *rechts)
    : Ausdruck(Ausdruck::BIN, spanne)
    , _op(op)
    , _links(links)
    , _rechts(rechts)
{
}

void
Ausdruck_Bin::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _links->ausgeben(tiefe+1, ausgabe);
    Einschub_Ausgeben(tiefe+1, ausgabe);
    ausgabe << Token_Namen(_op->art()) << std::endl;
    _rechts->ausgeben(tiefe+1, ausgabe);
    ausgabe << std::endl;
}

Ausdruck *
Ausdruck_Bin::kopie()
{
    Ausdruck_Bin *erg = new Ausdruck_Bin(spanne(), _op, _links->kopie(), _rechts->kopie());

    return erg;
}

Ausdruck_Name::Ausdruck_Name(Spanne spanne, std::string name)
    : Ausdruck(Ausdruck::NAME, spanne)
    , _name(name)
{
}

void
Ausdruck_Name::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _name << std::endl;
}

Ausdruck *
Ausdruck_Name::kopie()
{
    Ausdruck_Name *erg = new Ausdruck_Name(spanne(), _name);

    return erg;
}

std::string
Ausdruck_Name::name()
{
    return _name;
}

Ausdruck_Reg::Ausdruck_Reg(Spanne spanne, std::string name)
    : Ausdruck(Ausdruck::REG, spanne)
    , _name(name)
{
}

void
Ausdruck_Reg::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _name << std::endl;
}

Ausdruck *
Ausdruck_Reg::kopie()
{
    Ausdruck_Reg *erg = new Ausdruck_Reg(spanne(), _name);

    return erg;
}

uint32_t
Ausdruck_Reg::reg()
{
    auto erg = Register_Id(_name.c_str());

    return erg;
}

Ausdruck_Text::Ausdruck_Text(Spanne spanne, std::string text)
    : Ausdruck(Ausdruck::TEXT, spanne)
    , _text(text)
{
}

void
Ausdruck_Text::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _text << std::endl;
}

Ausdruck *
Ausdruck_Text::kopie()
{
    Ausdruck_Text *erg = new Ausdruck_Text(spanne(), _text);

    return erg;
}

std::string
Ausdruck_Text::text()
{
    return _text;
}

#if 0
Ganzzahl::Ganzzahl(Token *token)
    : Knoten(AST_GANZZAHL)
    , _token(token)
{
}

void
Ganzzahl::ausgeben(uint8_t tiefe)
{
    Einschub_Ausgeben(tiefe);
    printf("%s: %s\n", Ast_Namen(_art), _token->text());
}

uint32_t
Ganzzahl::wert()
{
    auto erg = _token->als<Token_Ganzzahl *>()->zahl();

    return erg;
}
#endif

Ausdruck_Klammer::Ausdruck_Klammer(Spanne spanne, Ausdruck *ausdruck)
    : Ausdruck(Ausdruck::KLAMMER, spanne)
    , _ausdruck(ausdruck)
{
}

Ausdruck *
Ausdruck_Klammer::ausdruck()
{
    return _ausdruck;
}

void
Ausdruck_Klammer::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _ausdruck->ausgeben(tiefe + 1, ausgabe);
    ausgabe << std::endl;
}

Ausdruck *
Ausdruck_Klammer::kopie()
{
    Ausdruck_Klammer *erg = new Ausdruck_Klammer(spanne(), _ausdruck->kopie());

    return erg;
}

Ausdruck_Adresse::Ausdruck_Adresse(Spanne spanne, Ausdruck *ausdruck)
    : Ausdruck(Ausdruck::ADRESSE, spanne)
    , _ausdruck(ausdruck)
{
}

void
Ausdruck_Adresse::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _ausdruck->ausgeben(tiefe + 1, ausgabe);
}

Ausdruck *
Ausdruck_Adresse::kopie()
{
    Ausdruck_Adresse *erg = new Ausdruck_Adresse(spanne(), _ausdruck->kopie());

    return erg;
}

Ausdruck *
Ausdruck_Adresse::ausdruck()
{
    return _ausdruck;
}

Ausdruck_Auswertung::Ausdruck_Auswertung(Spanne spanne, Ausdruck *ausdruck)
    : Ausdruck(Ausdruck::AUSWERTUNG, spanne)
    , _ausdruck(ausdruck)
{
}

void
Ausdruck_Auswertung::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _ausdruck->ausgeben(tiefe+1, ausgabe);
}

Ausdruck *
Ausdruck_Auswertung::kopie()
{
    Ausdruck_Auswertung *erg = new Ausdruck_Auswertung(spanne(), _ausdruck->kopie());

    return erg;
}

Ausdruck *
Ausdruck_Auswertung::ausdruck()
{
    return _ausdruck;
}

Ausdruck_Variable::Ausdruck_Variable(Spanne spanne, Ausdruck *ausdruck)
    : Ausdruck(Ausdruck::VARIABLE, spanne)
    , _ausdruck(ausdruck)
{
}

void
Ausdruck_Variable::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;
    _ausdruck->ausgeben(tiefe+1, ausgabe);
}

Ausdruck *
Ausdruck_Variable::kopie()
{
    Ausdruck_Variable *erg = new Ausdruck_Variable(spanne(), _ausdruck);

    return erg;
}

Ausdruck *
Ausdruck_Variable::ausdruck()
{
    return _ausdruck;
}

Ausdruck_Hex::Ausdruck_Hex(Spanne spanne, uint16_t wert)
    : Ausdruck(Ausdruck::HEX, spanne)
    , _wert(wert)
{
}

uint16_t
Ausdruck_Hex::wert()
{
    return _wert;
}

void
Ausdruck_Hex::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << " " << std::format("{:#06x}", wert()) << std::endl;
}

Ausdruck *
Ausdruck_Hex::kopie()
{
    Ausdruck_Hex *erg = new Ausdruck_Hex(spanne(), _wert);

    return erg;
}

Ausdruck_Ganzzahl::Ausdruck_Ganzzahl(Spanne spanne, uint16_t wert, uint16_t basis)
    : Ausdruck(Ausdruck::GANZZAHL, spanne)
    , _wert(wert)
    , _basis(basis)
{
}

uint16_t
Ausdruck_Ganzzahl::wert() const
{
    return _wert;
}

uint16_t
Ausdruck_Ganzzahl::basis() const
{
    return _basis;
}

void
Ausdruck_Ganzzahl::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << " " << std::format("{}b{}", _basis, _wert) << std::endl;
}

Ausdruck *
Ausdruck_Ganzzahl::kopie()
{
    Ausdruck_Ganzzahl *erg = new Ausdruck_Ganzzahl(spanne(), _wert, _basis);

    return erg;
}

Ausdruck_Feld::Ausdruck_Feld(Spanne spanne, Ausdruck *basis, std::string feld)
    : Ausdruck(Ausdruck::FELD, spanne)
    , _basis(basis)
    , _feld(feld)
{
}

void
Ausdruck_Feld::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << std::endl;

    Einschub_Ausgeben(tiefe+1, ausgabe);
    ausgabe << "Basis" << std::endl;

    _basis->ausgeben(tiefe+1, ausgabe);

    Einschub_Ausgeben(tiefe+1, ausgabe);
    ausgabe << "Feld: " << _feld;
}

Ausdruck *
Ausdruck_Feld::kopie()
{
    Ausdruck_Feld *erg = new Ausdruck_Feld(spanne(), _basis, _feld);

    return erg;
}

Ausdruck *
Ausdruck_Feld::basis()
{
    return _basis;
}

std::string
Ausdruck_Feld::feld()
{
    return _feld;
}
// }}}
// anweisung {{{
Anweisung::Anweisung(Anweisung::Art art, Spanne spanne)
    : _art(art)
    , _spanne(spanne)
    , _adresse(0)
    , _vm_anweisung(nullptr)
{
}

Anweisung::Art
Anweisung::art() const
{
    return _art;
}

Spanne
Anweisung::spanne() const
{
    return _spanne;
}

Vm::Anweisung *
Anweisung::vm_anweisung()
{
    return _vm_anweisung;
}

void
Anweisung::vm_anweisung_setzen(Vm::Anweisung *vm_anweisung)
{
    _vm_anweisung = vm_anweisung;
}

uint16_t
Anweisung::adresse()
{
    return _adresse;
}

void
Anweisung::adresse_setzen(uint16_t adresse)
{
    _adresse = adresse;
}

template<typename T>
T Anweisung::als()
{
    return static_cast<T> (this);
}

Anweisung_Asm::Anweisung_Asm(Spanne spanne, std::string op, std::vector<Ausdruck *> operanden)
    : Anweisung(Anweisung::ASM, spanne)
    , _op(op)
    , _operanden(operanden)
{
}

Anweisung *
Anweisung_Asm::kopie()
{
    std::vector<Ausdruck *> ops;

    for (auto *operand : operanden())
    {
        ops.push_back(operand->kopie());
    }

    Anweisung_Asm *erg = new Anweisung_Asm(spanne(), op(), ops);

    return erg;
}

void
Anweisung_Asm::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << _op << std::endl;

    for (auto op : _operanden)
    {
        op->ausgeben(tiefe + 1, ausgabe);
    }
}

uint32_t
Anweisung_Asm::größe()
{
    uint32_t erg = 1;

    for (auto *op : operanden())
    {
        if (op->art() == Ausdruck::REG)
        {
            erg += 1;
        }

        else if (op->art() == Ausdruck::ADRESSE)
        {
            erg += 2;
        }

        else if (op->art() == Ausdruck::AUSWERTUNG)
        {
            erg += 2;
        }

        else if (op->art() == Ausdruck::HEX)
        {
            erg += 2;
        }

        else {
            assert(!"unbekannte art des operanden");
        }
    }

    return erg;
}

std::string
Anweisung_Asm::op()
{
    return _op;
}

std::vector<Ausdruck *>
Anweisung_Asm::operanden()
{
    return _operanden;
}

Anweisung_Makro::Anweisung_Makro(Spanne spanne, std::string name, std::vector<Ausdruck *> argumente)
    : Anweisung(Anweisung::MAKRO, spanne)
    , _name(name)
    , _argumente(argumente)
{
}

void
Anweisung_Makro::ausgeben(uint8_t tiefe, std::ostream& ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _name << std::endl;
    Einschub_Ausgeben(tiefe+1, ausgabe);
    ausgabe << "Operanden" << std::endl;

    for (auto *argument : _argumente)
    {
        argument->ausgeben(tiefe+2, ausgabe);
    }
}

Anweisung *
Anweisung_Makro::kopie()
{
    std::vector<Ausdruck *> argumente;

    for (auto *argument : _argumente)
    {
        argumente.push_back(argument->kopie());
    }

    Anweisung_Makro *erg = new Anweisung_Makro(spanne(), _name, argumente);

    return erg;
}

std::string
Anweisung_Makro::name() const
{
    return _name;
}

std::vector<Ausdruck *>
Anweisung_Makro::argumente()
{
    return _argumente;
}

Anweisung_Markierung::Anweisung_Markierung(Spanne spanne, std::string name)
    : Anweisung(Anweisung::MARKIERUNG, spanne)
    , _name(name)
{
}

void
Anweisung_Markierung::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _name;
}

Anweisung *
Anweisung_Markierung::kopie()
{
    Anweisung_Markierung *erg = new Anweisung_Markierung(spanne(), _name);

    return erg;
}

std::string
Anweisung_Markierung::name()
{
    return _name;
}

Anweisung_Import::Anweisung_Import(Spanne spanne, std::string zone, uint16_t start_adresse, std::string modul)
    : Anweisung(Anweisung::IMPORT, spanne)
    , _zone(zone)
    , _start_adresse(start_adresse)
    , _modul(modul)
{
}

void
Anweisung_Import::ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    Einschub_Ausgeben(tiefe, ausgabe);
    ausgabe << Ast_Namen(art()) << ": " << _modul;
}

Anweisung *
Anweisung_Import::kopie()
{
    Anweisung_Import *erg = new Anweisung_Import(spanne(), _zone, _start_adresse, _modul);

    return erg;
}

std::string
Anweisung_Import::zone()
{
    return _zone;
}

uint16_t
Anweisung_Import::start_adresse()
{
    return _start_adresse;
}

std::string
Anweisung_Import::modul()
{
    return _modul;
}
// }}}

}
// helfer {{{
char *
Ast_Namen(Asm::Deklaration::Art art)
{
    switch (art)
    {
#define X(Name, Wert, Beschreibung) case Asm::Deklaration::Name: return (char *) Beschreibung;
        Deklaration_Art
#undef X
    }

    return (char *) "Deklaration: Unbekannt";
}

char *
Ast_Namen(Asm::Ausdruck::Art art)
{
    switch (art)
    {
#define X(Name, Wert, Beschreibung) case Asm::Ausdruck::Name: return (char *) Beschreibung;
        Ausdruck_Art
#undef X
    }

    return (char *) "Ausdruck: Unbekannt";
}

char *
Ast_Namen(Asm::Anweisung::Art art)
{
    switch (art)
    {
#define X(Name, Wert, Beschreibung) case Asm::Anweisung::Name: return (char *) Beschreibung;
        Anweisung_Art
#undef X
    }

    return (char *) "Ausdruck: Unbekannt";
}

uint32_t
Register_Id(const char * name)
{
    if (strcmp(name, "r1")   == 0 || strcmp(name, "R1")  == 0) return Vm::REG_R1;
    if (strcmp(name, "r2")   == 0 || strcmp(name, "R2")  == 0) return Vm::REG_R2;
    if (strcmp(name, "r3")   == 0 || strcmp(name, "R3")  == 0) return Vm::REG_R3;
    if (strcmp(name, "r4")   == 0 || strcmp(name, "R4")  == 0) return Vm::REG_R4;
    if (strcmp(name, "r5")   == 0 || strcmp(name, "R5")  == 0) return Vm::REG_R5;
    if (strcmp(name, "r6")   == 0 || strcmp(name, "R6")  == 0) return Vm::REG_R6;
    if (strcmp(name, "r7")   == 0 || strcmp(name, "R7")  == 0) return Vm::REG_R7;
    if (strcmp(name, "r8")   == 0 || strcmp(name, "R8")  == 0) return Vm::REG_R8;
    if (strcmp(name, "sp")   == 0 || strcmp(name, "SP")  == 0) return Vm::REG_SP;
    if (strcmp(name, "fp")   == 0 || strcmp(name, "FP")  == 0) return Vm::REG_FP;
    if (strcmp(name, "ip")   == 0 || strcmp(name, "IP")  == 0) return Vm::REG_IP;
    if (strcmp(name, "acu")  == 0 || strcmp(name, "ACU") == 0) return Vm::REG_ACU;

    return 0;
}

void Einschub_Ausgeben(uint8_t tiefe, std::ostream &ausgabe)
{
    for (int32_t i = 0; i < tiefe*3; ++i)
    {
        ausgabe << " ";
    }
}
// }}}
