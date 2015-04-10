#include <iostream>
#include "unittest.h"
#include "list.h"

using std::cout;
using std::endl;

void RunUnitTests() {

	cout << endl << "--- Running Unit Tests ---" << endl;

	ListUnitTests();


}

void PrintNums(int d) {

	cout << *((int*) d) << endl;

}

void CheckFive(int d) {
	ASSERT(*((int*) d) != 5);
}

void ListUnitTests() {

	List* list = new List();
	for (int i = 1; i <= 4; i++) {
		list->Append((void * ) new int(i));
	}
	ListElement* save = list->Append((void*) new int(5));
	for (int i = 6; i <= 10; i++) {
		list->Append((void *) new int(i));
	}
	
	list->Remove(save);

	list->Mapcar(CheckFive);
	cout << "List Removed: PASSED" << endl;	
}


