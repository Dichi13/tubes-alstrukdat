#include "unit.h"
#include <stdio.h>
#include <stdlib.h>
#include "mesinkata.h"
#include <time.h>

/* Deklarasi variabel global */
UnitType *TypeList;
int NbUnitType;

void InitUnitTypeList() {
	int i = 0;
	STARTKATA("unittypes.txt");
	ADVKATA();
	NbUnitType = KataInt(CKata);
	TypeList = (UnitType *) malloc(NbUnitType*sizeof(UnitType));
	ADVKATA();
	while (!EndKata) {
		ADVKATA(); ADVKATA();
		TypeList[i].TypeName = KataStr(CKata);
		
		ADVKATA(); ADVKATA();
		TypeList[i].MaxHP = KataInt(CKata);
		
		ADVKATA(); ADVKATA();
		TypeList[i].Atk = KataInt(CKata);
		
		ADVKATA(); ADVKATA();
		TypeList[i].MaxMove = KataInt(CKata);
		
		ADVKATA(); ADVKATA();
		TypeList[i].AtkType = KataStr(CKata)[0];
		
		ADVKATA(); ADVKATA();
		TypeList[i].Cost = KataInt(CKata);
		
		ADVKATA();
		i++;
	}
}
/* I.S. TypeList sembarang, terdapat file "unittypes.txt" di folder yang sama, dengan format isi file yang valid */
/* F.S. TypeList terdefinisi dan terisi dengan tipe-tipe unit yang digunakan dalam game */ 

Unit CreateUnit(int IdxList, POINT P, int ownerNo) {
	Unit temp;
	
	TypeID(temp) = IdxList;
	OwnerUnit(temp) = ownerNo;
	Health(temp) =  TypeList[IdxList].MaxHP;
	MovePoint(temp) = 0;
	CanAtk(temp) = false;
	Position(temp) = P;
	
	return temp;
}
/* Membuat unit baru dengan tipe unit yang terdapat pada TypeList[IdxList] */
/* Unit yang baru dibuat tidak dapat bergerak dan tidak bisa menyerang */
/* Catatan: IdxList harus sesuai dengan ID unit di unittypes.txt */

void MoveUnit(Unit *U, int dx, int dy){
	MovePoint(*U) -= abs(dx) + abs(dy);
	Geser(&Position(*U), dx, dy);
}
/* I.S. U terdefinisi, dx dan dy valid */
/* F.S. Menggeser lokasi unit sejauh (dx,dy) */

boolean IsAdjacent(Unit U1, Unit U2) {
	return (abs(Absis(Position(U1))-Absis(Position(U2))) + abs(Ordinat(Position(U1))-Ordinat(Position(U2)))) == 1;
}
/* mengembalikan true jika kedua unit berjarak satu petak. */

boolean CanRetaliate(Unit U1, Unit U2)
{
	return ((AtkType(U1) == AtkType(U2)) || StrSama(TypeName(U2), "King"));
}
/* Menghasilkan true jika tipe attack Unit U2 dapat retaliate jika diserang U1 */

void UnreadyUnit(Unit *U)
{
	MovePoint(*U) = 0;
	CanAtk(*U) = false;
}
/* I.S. Unit terdefinisi */
/* F.S. MovePoint unit menjadi 0 dan boolean Actionnya false */



void RefreshUnit(Unit *U){
	MovePoint(*U) = MaxMove(*U);
	CanAtk(*U) = true;
}
/* I.S. U terdefinisi */
/* F.S MovePoint(U) berisi move point maksimum dan CanAtk(U) bernilai true */

void HealUnit(Unit *U, int heal) {
	if ((Health(*U) + heal) > MaxHP(*U)) {
		Health(*U) = MaxHP(*U);
	} else {
		Health(*U) += heal;
	}
}
/* I.S. U terdefinisi */
/* F.S Health(U) bertambah sebanyak heal jika tidak melebihi MaxHP(U), jika melebihi, 
 * Health(U) menjadi bernilai MaxHP(U) */ 
