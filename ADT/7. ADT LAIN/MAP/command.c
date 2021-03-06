/* Command */

#include "command.h"
#include "unit.h"
#include "map.h"
#include <stdio.h>
#include <math.h>
#include "point.h"
#include "stack.h"
#include "player.h"
#include "turn.h"
#include <math.h>
#include "pcolor.h"

boolean EndGame;
int Winner;

boolean IsPathClear(POINT src, POINT dest, Unit U)
{
	/* KAMUS LOKAL */
	POINT Px, Py;
	boolean ClearX, ClearY;
	
	/* ALGORITMA */
	
	if (EQ(src, dest)){
		return true;
	}
	else{
		Px = src;
		Py = src;
		
		if (Absis(src) < Absis(dest)){
			Geser(&Px, 1, 0);
		}
		if (Absis(src) > Absis(dest)){
			Geser(&Px, -1, 0);
		}
		if (Ordinat(src) < Ordinat(dest)){
			Geser(&Py, 0, 1);
		}
		if (Ordinat(src) > Ordinat(dest)){
			Geser(&Py, 0, -1);
		}
		
		if(EQ(Px, src)){
			ClearX = false;
		}
		else{
			if((PlotUnit(Petak(M, Px)) != Nil) && OwnerUnit(*PlotUnit(Petak(M, Px))) != OwnerUnit(U)){
				ClearX = false;
			}
			else{
				ClearX = IsPathClear(Px, dest, U);
			}
		}
		if(EQ(Py, src)){
				ClearY = false;
		}
		else{
			if((PlotUnit(Petak(M, Py)) != Nil) && OwnerUnit(*PlotUnit(Petak(M, Py))) != OwnerUnit(U)){
				ClearY = false;
			}
			else{
				ClearY = IsPathClear(Py, dest, U);
			}
		}
		
		return (ClearX || ClearY);
	}
	
}
/* Mengembalikan true jika path dari src menuju dest memiliki jalur yang tidak dihalangi musuh */
/* Dipastikan unit U dapat jalan ke dest dengan move pointnya jika tidak dihalangi */

boolean IsMoveValid(POINT dest, Unit U)
{
	/* KAMUS LOKAL */
	int dx, dy;
	POINT current;
	boolean valid, pathclear;
	
	/* ALGORITMA */
	current = Position(U);
	dx = abs(Absis(current) - Absis(dest));
	dy = abs(Ordinat(current) - Ordinat(dest));
	valid = ((dx + dy) <= MovePoint(U));
	
	if (valid) {
		valid =  IsIdxEff(Peta(M), Absis(dest), Ordinat(dest));
		if(valid && PlotUnit(Petak(M, dest)) != Nil){
			valid = false;
		}  
	}
	
	pathclear = IsPathClear(Position(U), dest, U);
	
	return valid && pathclear;
}

void Move()
{
	/* KAMUS LOKAL */
	POINT dest;
	
	/* ALGORITMA */
	PrintMapMove();
	printf("Please enter destination coordinate x y (example : 1 1 ) : ");
	BacaPOINT(&dest);
	printf("\n");
	/* geser sejauh dx,dy */
	if (IsMoveValid(dest, *SelectedUnit)){
		Push(Position(*SelectedUnit));
		SetUnit(Position(*SelectedUnit), Nil);
		MoveUnit(SelectedUnit, Absis(dest) - Absis(Position(*SelectedUnit)), Ordinat(dest) - Ordinat(Position(*SelectedUnit)));
		SetUnit(dest, SelectedUnit);
		
		/* Pengambilan Village */
		if((PlotType(Petak(M, dest)) == 'V') && (Owner(Petak(M, dest)) != PlayerNo(*currPlayer))){
			if(Owner(Petak(M, dest)) != 0){
				DelVillage(SearchPlayer(Owner(Petak(M, dest))), &dest);
				Income(*SearchPlayer(Owner(Petak(M, dest)))) -= IncomePerVillage;
			}
			AddVillage(currPlayer, dest);
			Income(*currPlayer) += IncomePerVillage;
			SetPlot(dest, 'V', PlayerNo(*currPlayer));
		}
		
		printf("Your %s has moved to ", TypeName(*SelectedUnit));
		TulisPOINT(dest);
		printf("\n");
	}
	else
	{
		printf("You can't move there\n");
	}
}

void ChangeUnit()
{
	/* KAMUS LOKAL */
	int NBUnit, selection;
	addressUnit U;
	
	/* ALGORITMA */
	PrintUnitPlayer(*currPlayer);
	NBUnit = NbElmtListUnit(*currPlayer);
	do {
		printf("Please enter your selection number : ");
		scanf("%d", &selection);
		if ((selection > NBUnit) || (selection <= 0)){
			printf("Please enter a valid number\n");
		}
	} while ((selection > NBUnit) || (selection <= 0));
	
	U = TraversalElmtUnitList(*currPlayer, selection);
	
	SelectedUnit = &InfoUnit(U);
	SetUnit(Position(*SelectedUnit), SelectedUnit);
	
	printf("You are now selecting %s (%d,%d)\n", TypeName(*SelectedUnit), Absis(Position(*SelectedUnit)), Ordinat(Position(*SelectedUnit)));
}

void NextSelect()
{
	/* KAMUS LOKAL */
	boolean found;
	addressUnit U;
	
	/* ALGORITMA */
	U = ListUnit(*currPlayer);
	found = false;
	
	while(U != Nil && !found){
		found = (MovePoint(InfoUnit(U)) > 0 || CanAtk(InfoUnit(U)));
		if (!found){
			U = NextUnit(U);
		}
	}

	if (!found){
		printf("None of your units are still able to act\n");
	}
	else{	
		SelectedUnit = &InfoUnit(U);
		SetUnit(Position(*SelectedUnit), SelectedUnit);
		printf("You are now selecting %s (%d,%d)\n", TypeName(*SelectedUnit), Absis(Position(*SelectedUnit)), Ordinat(Position(*SelectedUnit)));
	}
	
}
void Undo() 
{
	/* Kamus Lokal */
	POINT PosAwal;
	int dist;
	
	/* Algoritma */
	Pop(&PosAwal);
	dist = abs(Absis(Position(*SelectedUnit))-Absis(PosAwal)) + abs(Ordinat(Position(*SelectedUnit))-Ordinat(PosAwal));
	MovePoint(*SelectedUnit) += dist;
	SetUnit(Position(*SelectedUnit), Nil);
	Position(*SelectedUnit) = PosAwal;
	SetUnit(PosAwal, SelectedUnit);	
	CanAtk(*SelectedUnit) = true;
	
	printf("Your %s previous move has been undone\n", TypeName(*SelectedUnit));
}

boolean IsCastleFull()
{
	/* KAMUS LOKAL */
	boolean Full;
	POINT P;
	POINT C1, C2, C3, C4;
	
	/* ALGORITMA */
	if(PlayerNo(*currPlayer) == 1){
	/* Inisiasi untuk Player 1 */
		P = TowerCoordinate(1);
	}
	if(PlayerNo(*currPlayer) == 2){
	/* Inisiasi untuk Player 2 */
		P = TowerCoordinate(2);
	}
	
	C1 = PlusDelta(P, -1, 0);
	C2 = PlusDelta(P, 1, 0);
	C3 = PlusDelta(P, 0, 1);
	C4 = PlusDelta(P, 0, -1);
		
	Full = ((PlotUnit(Petak(M, C1)) != Nil) && (PlotUnit(Petak(M, C2)) != Nil) && (PlotUnit(Petak(M, C3)) != Nil) && (PlotUnit(Petak(M, C4)) != Nil));
	
	return Full;	
}


void Recruit()
{
	/* KAMUS LOKAL */
	POINT dest;
	boolean valid;
	int i;
	Unit U;
	
	/* ALGORITMA */
	if (strcmp(TypeName(*SelectedUnit),"King") == 0){
		if(PlotType(Petak(M, Position(*SelectedUnit))) == 'T'){
			if(!IsCastleFull()){
				do{
					printf("Enter coordinate of empty castle : ");
					BacaPOINT(&dest);
					valid = true;
					if(PlotType(Petak(M, dest)) != 'C'){
						valid = false;
						printf("Selected coordinate is not a castle!\n");
					}
					else{
						if(Owner(Petak(M, dest)) != PlayerNo(*currPlayer)){
							valid = false;
							printf("That is not your castle!\n");
						}
					}
				} while (!valid);
				printf("\n========== List of recruitable units ==========\n");
				for (i = 1; i<NbUnitType; i++) {
					printf("%d. %s | Health %d | ATK %d | Cost %dG\n", i, TypeList[i].TypeName, TypeList[i].MaxHP, TypeList[i].Atk, TypeList[i].Cost);
				}
				printf("Enter the index of the unit you want to recruit :");
				scanf("%d", &i);
				if (Gold(*currPlayer) >= TypeList[i].Cost) {
					printf("You have recruited a");
					if((TypeList[i].TypeName[0] == 'A') || (TypeList[i].TypeName[0] == 'I') || (TypeList[i].TypeName[0] == 'U') || (TypeList[i].TypeName[0] == 'E') || (TypeList[i].TypeName[0] == 'O')){
						printf("n");
					}
					printf(" %s!\n", TypeList[i].TypeName);
					Gold(*currPlayer) -= TypeList[i].Cost;
					U = CreateUnit(i, dest, PlayerNo(*currPlayer));
					AddUnit(currPlayer, U);
					UnreadyUnit(&U);
				} else {
					printf("You don't have enough gold to recruit that unit!\n");
				}
				printf("\n");
				
			}
			else{
				printf("All castles are full");
			}
		}
		else{
			printf("The King is not in the Tower\n");
		} 
	}
	else{
		printf("You have to select The King to recruit\n");
	}
}

boolean IsAdjacentEmpty(Unit U, boolean Enemy)
{
	/* KAMUS LOKAL */
	boolean found;
	Unit* UAd;
	int i;
	
	/* ALGORITMA */
	if (Enemy){
		i = 0;
		while (i < 4 && !found){
			i++;
			UAd = ChooseAdjacentUnit(U, i);
			found = ((UAd != Nil) && (OwnerUnit(*UAd) != PlayerNo(*currPlayer)));
		}
	}
	else{
		i = 0;
		while (i < 4 && !found){
			i++;
			UAd = ChooseAdjacentUnit(U, i);
			found = ((UAd != Nil) && (OwnerUnit(*UAd) == PlayerNo(*currPlayer)));
		}
	}
	
	return !found;
}
/* Menghasilkan True jika ada Unit untuk tipe Enemy di sekitar unit (Kiri/Atas/Kanan/Bawah) 
 * Mengecek keberadaan Musuh jika Enemy = true, dan sebaliknya */

void ShowAttackTarget(Unit U)
{
	/* KAMUS LOKAL */
	Unit* UAd;
	int i, count;
	
	/* ALGORITMA */
	count = 0;
	for(i = 1; i <= 4; i++){
		UAd = ChooseAdjacentUnit(U, i);
		if (UAd != Nil){
			if (OwnerUnit(*UAd) != PlayerNo(*currPlayer)){
				count += 1;
				printf("%d. ",count);
				printf("%s ", TypeName(*UAd));
				printf("(%d,%d) | Health %d/%d | ATK %d", Absis(Position(*UAd)), Ordinat(Position(*UAd)), Health(*UAd), MaxHP(*UAd), Atk(*UAd));
				if(CanRetaliate(U, *UAd)){
					printf(" (Retaliates)\n");
				}
				else{
					printf(" (Does not Retaliate)\n");
				}
			}			
		}
	}
}

void Attack()
{
	/* KAMUS LOKAL */
	int i, count, selection, InitHP1, InitHP2;
	Unit* UAd;
	
	/* ALGORITMA */
	if (CanAtk(*SelectedUnit)){
		if (IsAdjacentEmpty(*SelectedUnit, true)){
				printf("There are no enemies adjacent to selected Unit\n");
		}
		else{
			printf("Please select the enemy unit you want to attack\n");
			ShowAttackTarget(*SelectedUnit);
			do{
				printf("Your selection : ");
				scanf("%d", &selection);
				count = 0;
				i = 0;
				while ((i < 4) && (count != selection)){
					i++;
					UAd = ChooseAdjacentUnit(*SelectedUnit, i);
					if (UAd != Nil){
						if (OwnerUnit(*UAd) != PlayerNo(*currPlayer)){
							count++;
						}
					}
				}
				if(count != selection){
					printf("Please select a valid target\n");
				}
			}while (count != selection);
			InitHP1 = Health(*SelectedUnit);
			InitHP2 = Health(*UAd);
			
			AttackUnit(SelectedUnit, UAd);
			
			if (Health(*UAd) == InitHP2){
				printf("Your %s's attack missed\n", TypeName(*SelectedUnit));
			}
			else{ 
				printf("Enemy's %s is damaged by %d.\n", TypeName(*UAd), Atk(*SelectedUnit));
				if (Health(*UAd) <= 0){
					printf("Enemy's %s died.\n", TypeName(*UAd));
					if (StrSama(TypeName(*UAd), "King")){
						EndGame = true;
						Winner = OwnerUnit(*UAd);
					}
					SetUnit(Position(*UAd), Nil);
					DelUnit(SearchPlayer(OwnerUnit(*UAd)), UAd);
					UnreadyUnit(SelectedUnit);
				}
				else{
					if (CanRetaliate(*SelectedUnit, *UAd)){
						printf("Enemy's %s retaliates.\n", TypeName(*UAd));
						if(Health(*SelectedUnit) == InitHP1){
							printf("Enemy's %s's attack missed.\n", TypeName(*UAd));
						}
						else{
							printf("Your %s is damaged by %d.\n", TypeName(*SelectedUnit), Atk(*UAd));
							if(Health(*SelectedUnit) <= 0){
								printf("Your %s died.\n", TypeName(*SelectedUnit));
								if (StrSama(TypeName(*SelectedUnit), "King")){
									EndGame = true;
									Winner = OwnerUnit(*SelectedUnit);
								}
								SetUnit(Position(*SelectedUnit), Nil);
								DelUnit(currPlayer, SelectedUnit);
								SelectedUnit = Nil;
							}
							else{
								UnreadyUnit(SelectedUnit);
							}
						}
					}
				}
			}
		}
	}
	else{
		printf("This unit already attacked or has just been recruited\n");
	}
}


/* Mencetak semua Unit yang jarak dengan Unit U = 1 
 * Menunjukan Unit musuh jika Enemy = true, dan sebaliknya */
void InfoPetak()
{
	/* KAMUS LOKAL */
	POINT P;
	
	/* ALGORITMA */
	do{
		printf("Please enter cell's coordinate x y : ");
		BacaPOINT(&P);
		if (IsIdxEff((Peta(M)), Absis(P), Ordinat(P))){
			printf("== Plot Info ==\n");
			printf("Plot type of ");
			PrintPlotType(P);
			printf("\n");
			if (Owner(Petak(M, P)) != 0){
				printf("Owned by Player %d\n", Owner(Petak(M, P)));
			}
			else{
				printf("Plot not owned by any player\n");
			}
			printf("\n== Unit Info ==\n");
			if (PlotUnit(Petak(M, P)) != Nil){
				printf("%s\n", TypeName(*PlotUnit(Petak(M, P))));
				printf("Owned by Player %d\n", OwnerUnit(*PlotUnit(Petak(M, P))));
				printf("Health %d/%d | ATK %d", Health(*PlotUnit(Petak(M, P))), MaxHP(*PlotUnit(Petak(M, P))), Atk(*PlotUnit(Petak(M, P))));
			}
			else{
				printf("There is no unit inside this plot\n");
			}
		}
		else{
			printf("Invalid plot\n");
		}
	} while (!(IsIdxEff((Peta(M)), Absis(P), Ordinat(P))));
	printf("\n");
}

/* Input/Output */
void PrintMap()
{
	/* KAMUS LOKAL */
	int i, j;
	POINT P;
	
	/* ALGORITMA */
	printf("\n==-");
	for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
		printf("-%d--",j);	
	}
	printf("\n");
	for(i = GetFirstIdxBrs(Peta(M)); i <= GetLastIdxBrs(Peta(M)); i++){
		/* Print garis "*****" */
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			printf("****");	
		}
		printf("\n");
		
		/* Print jenis petak */
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			P = MakePOINT(i, j);
			if (PlotType(Petak(M, P)) != 'N'){
				printf(" ");
				if (Owner(Petak(M, P)) == 0){
					printf("%c", PlotType(Petak(M, P)));
				}
				if (Owner(Petak(M, P)) == 1){
					print_red(PlotType(Petak(M, P)));
				}
				if (Owner(Petak(M, P)) == 2){
					print_cyan(PlotType(Petak(M, P)));
				}
			}
			else{
				printf("  ");
			}
			printf(" *");
		}
		printf("\n");
		
		/* Print Unit */
		printf("%d *", i);
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			P = MakePOINT(i, j);
			if (PlotUnit(Petak(M, P)) != Nil){
				printf(" ");
				if (SelectedUnit == PlotUnit(Petak(M, P))){
					print_green(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
				else if (OwnerUnit(*PlotUnit(Petak(M, P))) == 1){
					print_red(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
				else if (OwnerUnit(*PlotUnit(Petak(M, P))) == 2){
					print_cyan(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
			}
			else{
				printf("  ");
			}
			printf(" *");
		}
		printf("\n");
		
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			printf("   *");	
		}
		printf("\n");
		
	}
	printf("| *");
	for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
		printf("****");	
	}
	printf("\n");
}	
/* Map terdefinisi */
/* Map dicetak pada layar dengan format
 * ***** 
 * *​ ​​X ​* 
 * *​ ​​Y ​​* 
 * *   *
 * *****
 * Dengan X adalah jenis petak, dikosongkan jika petak Normal
 * Y adalah jenis unit pada petak, dikosongkan jika tidak ada unit */


void PrintMapMove()
{
	/* KAMUS LOKAL */
	int i, j;
	POINT P;
	
	/* ALGORITMA */
	printf("\n==-");
	for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
		printf("-%d--",j);	
	}
	printf("\n");	
	for(i = GetFirstIdxBrs(Peta(M)); i <= GetLastIdxBrs(Peta(M)); i++){
		/* Print garis "*****" */
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			printf("****");	
		}
		printf("\n");
		
		/* Print jenis petak */
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			P = MakePOINT(i, j);
			if (PlotType(Petak(M, P)) != 'N'){
				printf(" ");
				if (Owner(Petak(M, P)) == 0){
					printf("%c", PlotType(Petak(M, P)));
				}
				if (Owner(Petak(M, P)) == 1){
					print_red(PlotType(Petak(M, P)));
				}
				if (Owner(Petak(M, P)) == 2){
					print_cyan(PlotType(Petak(M, P)));
				}
			}
			else{
				printf("  ");
			}
			printf(" *");
		}
		printf("\n");
		
		/* Print Unit */
		printf("%d *", i);
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			P = MakePOINT(i, j);
			if (PlotUnit(Petak(M, P)) != Nil){
				printf(" ");
				if (SelectedUnit == PlotUnit(Petak(M, P))){
					print_green(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
				else if (OwnerUnit(*PlotUnit(Petak(M, P))) == 1){
					print_red(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
				else if (OwnerUnit(*PlotUnit(Petak(M, P))) == 2){
					print_cyan(TypeName(*PlotUnit(Petak(M, P)))[0]);
				}
			}
			else{
				if(IsMoveValid(P, *SelectedUnit)){
					printf(" #");
				}
				else{
					printf("  ");
				}
			}
			printf(" *");
		}
		printf("\n");
		
		printf("| *");
		for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
			printf("   *");	
		}
		printf("\n");
		
	}
	printf("| *");
	for(j = GetFirstIdxKol(Peta(M)); j <= GetLastIdxKol(Peta(M)); j++){
		printf("****");	
	}
	printf("\n");
}	
/* Map terdefinisi */
/* Map dicetak pada layar dengan format
 * ***** 
 * *​ ​​X ​* 
 * *​ ​​Y ​​* 
 * *   *
 * *****
 * Dengan X adalah jenis petak, dikosongkan jika petak Normal
 * Y adalah jenis unit pada petak, dikosongkan jika tidak ada unit */
/* Tercetak
 * ***** 
 * *​ ​​X ​* 
 * *​ ​​# ​​*
 * *   * 
 * *****
 * jika petak tersebut valid untuk jalan unit */



/* Combat Unit berdasarkan Petak */
boolean MissChance(Unit U1, Unit U2, boolean Attacking)
{
	/* KAMUS LOKAL */
	int Chance;
	int Random;
	time_t t;
	
	/* ALGORITMA */
	srand((unsigned) time(&t));
	
	Chance = 10;
	if(StrSama(TypeName(U1), "Archer")){
		Chance += 5;
	}
	if(StrSama(TypeName(U1), "King")){
		Chance -= 10;
	}
	if(!Attacking){
		Chance += 10;
	}
	if(PlotType(Petak(M, Position(U2))) == 'V'){
		Chance += 5;
	}
	if(PlotType(Petak(M, Position(U2))) == 'C'){
		Chance += 10;
	}
	if(PlotType(Petak(M, Position(U2))) == 'T'){
		Chance += 15;
	}
	
	Random = (rand() % 100)+1;
	
	return (Random < Chance);
}
/* Mengembalikan true jika kalkulasi misschance hasilnya true (Serangan unit U1 terhadap U2 miss) */
/* Attacking bernilai true jika Unit U1 memulai combat (bukan retaliate) */
/* Nilai misschance tiap unit berbeda */
/* Default MissChance adalah 10% */
/* Untuk unit tipe :
 * Archer : 15%
 * King : 0% */
/* Unit yang retaliate memiliki miss change lebih tinggi (20%) (Mendorong player untuk lebih aktif dan inisiatif) */
/* Unit mendapatkan bonus miss chance berdasarkan petak tempat unit itu berada : 
 * Normal plot : 0%
 * Village : 5%
 * Castle : 10%
 * Tower : 15% */

 
void AttackUnit(Unit *U1, Unit *U2) {
	if(!MissChance(*U1, *U2, true)){
		Health(*U2) = Health(*U2) - Atk(*U1);
		if (Health(*U2) > 0) {
			if (CanRetaliate(*U1, *U2)) {
				if(!MissChance(*U2, *U1, false)){
					Health(*U1) = Health(*U1) - Atk(*U2);
				}
			} 
		}
	}
}
/* I.S. U1 dan U2 terdefinisi, U1 memenuhi syarat untuk melakukan serangan ke U2 */
/* F.S. Melaksanakan serangan dari U1 ke U2 sesuai definisi "serangan" di spesifikasi tugas */
