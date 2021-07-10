/*
*	Name: Mihai Andrei
*	Group: 314CD
*/
#include <stdio.h>
#include "WearhouseManager.h"

Package *create_package(long priority, const char* destination){
	Package* package = (Package*) malloc(sizeof(Package));
	if(!package) 
		exit(EXIT_FAILURE);

	if(destination == NULL) 
		package->destination = NULL;
	else {
		package->destination = (char*) malloc(MAX_DESTINATION_NAME_LEN * 
												sizeof(char));
		if(!package->destination) 
			exit(EXIT_FAILURE);
		
		strcpy(package->destination, destination);
	}
	
	package->priority = priority;
	return package;
}

void destroy_package(Package* package){
	if(package) {
		if(package->destination) {
			free(package->destination);
			package->destination = NULL;
		}

		free(package);
		package = NULL;
	}
}

Manifest* create_manifest_node(void){
	Manifest* manifest_node = (Manifest*) malloc(sizeof(Manifest));
	if(!manifest_node) 
		exit(EXIT_FAILURE);
	
	manifest_node->package = NULL;
	manifest_node->next = NULL;
	manifest_node->prev = NULL;

	return manifest_node;
}

void destroy_manifest_node(Manifest* manifest_node){
	if(manifest_node) {
		destroy_package(manifest_node->package);

		manifest_node->prev = NULL;
		manifest_node->next = NULL;

		free(manifest_node);
		manifest_node = NULL;
	}
}

Wearhouse* create_wearhouse(long capacity){
	if(capacity == 0) 
		return NULL;
	
	Wearhouse* wearhouse = (Wearhouse*) malloc(sizeof(Wearhouse));
	if(!wearhouse)
		exit(EXIT_FAILURE);

	wearhouse->packages = (Package**) malloc(capacity * sizeof(Package*));
	if(!wearhouse->packages) 
		exit(EXIT_FAILURE);

	long i;
	for(i = 0; i < capacity; i++) 
		wearhouse->packages[i] = NULL;

	wearhouse->size = 0;
	wearhouse->capacity = capacity;

	return wearhouse;
}

Wearhouse *open_wearhouse(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Wearhouse *w = NULL;


	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	if((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		w = create_wearhouse(atol(token));

		free(line);
		line = NULL;
		len = 0;
	}

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		long priority = atol(token);
		token = strtok(NULL, ",\n ");
		Package *p = create_package(priority, token);
		w->packages[w->size++] = p;

		free(line);
		line = NULL;
		len = 0;
	}

	free(line);

	fclose(fp);
	return w;

	file_open_exception:
	return NULL;
}

long wearhouse_is_empty(Wearhouse *w){
	if(w) {
		if(w->size == 0 && w->capacity != 0)
			return 1;
	}

	return 0;
}

long wearhouse_is_full(Wearhouse *w){
	if(w) {
		if(w->size == w->capacity)
			return 1;
	}

	return 0;
}

long wearhouse_max_package_priority(Wearhouse *w){
	if((w == NULL) || (wearhouse_is_empty(w)))
		return -1;

	int i;
	long max_package_priority = w->packages[0]->priority;
	for(i = 0; i < w->size; i++) {
		if(max_package_priority < w->packages[i]->priority)
			max_package_priority = w->packages[i]->priority;
	}

	return max_package_priority;
}

long wearhouse_min_package_priority(Wearhouse *w){
	if((w == NULL) || (wearhouse_is_empty(w)))
		return -1;

	int i;
	long min_package_priority = w->packages[0]->priority;
	for(i = 0; i < w->size; i++) {
		if(min_package_priority > w->packages[i]->priority)
			min_package_priority = w->packages[i]->priority;
	}

	return min_package_priority;
}


void wearhouse_print_packages_info(Wearhouse *w){
	for(long i = 0; i < w->size; i++){
		printf("P: %ld %s\n",
				w->packages[i]->priority,
				w->packages[i]->destination);
	}
	printf("\n");
}

void destroy_wearhouse(Wearhouse* wearhouse){
	if(wearhouse) {
		int i;
		for(i = 0; i < wearhouse->size; i++)
			destroy_package(wearhouse->packages[i]);

		free(wearhouse->packages);
		free(wearhouse);
		wearhouse = NULL;
	}
}

Robot* create_robot(long capacity){
	Robot* robot = (Robot*) malloc(sizeof(Robot));
	if(!robot)
		exit(EXIT_FAILURE);

	robot->manifest = NULL;
	robot->size = 0;
	robot->capacity = capacity;

	return robot;
}

int robot_is_full(Robot* robot){
	if(robot) {
		if(robot->size == robot->capacity)
			return 1;
	}

	return 0;
}

int robot_is_empty(Robot* robot){
	if(robot) {
		if(robot->size == 0 && robot->capacity != 0) 
			return 1;
	}

	return 0;
}

Package* robot_get_wearhouse_priority_package(Wearhouse *w, long priority){
	if(!w)
		return NULL;

	int i;
	Package* package = NULL;
	for(i = 0; i < w->size; i++) {
		if(w->packages[i]->priority == priority) {
			package = w->packages[i];
			break;
		}
	}

	return package;
}

void robot_remove_wearhouse_package(Wearhouse *w, Package* package){
	if((wearhouse_is_empty(w)) || (!w) || (!package))
		return;

	int i;
	for(i = 0; i < w->size - 1; i++) {
		if(w->packages[i] == package) {
			int j;
			for(j = i; j < w->size - 1; j++) {
				w->packages[j] = w->packages[j + 1];
			}
			break;
		}
	}

	w->size--;
}

void robot_load_one_package(Robot* robot, Package* package){
	if(!robot) 
		return;

	if(!robot_is_full(robot)) {
		Manifest* new_Manifest = create_manifest_node();
		new_Manifest->package = package;

		if(robot_is_empty(robot)) {
			robot->manifest = new_Manifest;
			robot->size++;
			return;
		}

		Manifest* current_Manifest = robot->manifest;
		Manifest* prev_Manifest = NULL;

		while((current_Manifest != NULL) && 
			(current_Manifest->package->priority > package->priority || 
				((current_Manifest->package->priority == package->priority) && 
					(strcmp(package->destination, current_Manifest->package->destination) > 0)))) {

			prev_Manifest = current_Manifest;
			current_Manifest = current_Manifest->next;
		}

		if(current_Manifest == robot->manifest) {
			new_Manifest->next = robot->manifest;
			robot->manifest->prev = new_Manifest;
			robot->manifest = new_Manifest;
		} else if(current_Manifest == NULL) {
			new_Manifest->prev = prev_Manifest;
			prev_Manifest->next = new_Manifest;
		} else {
			new_Manifest->prev = prev_Manifest;
			new_Manifest->next = current_Manifest;

			prev_Manifest->next = new_Manifest;
			current_Manifest->prev = new_Manifest;
		}

		robot->size++;
	}
}

long robot_load_packages(Wearhouse* wearhouse, Robot* robot){
	if((!wearhouse) || (!robot))
		return 0;

	if(!robot_is_full(robot)) {
		long free_slots = robot->capacity - robot->size;
		long loaded_packages = 0;

		int i;
		for(i = 0; i < free_slots; i++) {
			long max_priority = wearhouse_max_package_priority(wearhouse);
			if(max_priority < 0) 
				return loaded_packages;

			Package* package_to_load = robot_get_wearhouse_priority_package(wearhouse, max_priority);
			robot_load_one_package(robot, package_to_load);
			robot_remove_wearhouse_package(wearhouse, package_to_load);

			loaded_packages++;
		}

		return loaded_packages;
	}

	return 0;
}

Package* robot_get_destination_highest_priority_package(Robot* robot, const char* destination) {
	if((!robot) || (robot_is_empty(robot)) || (destination == NULL))
		return NULL;

	Package* highest_priority_package = NULL;
	Manifest* current_Manifest = robot->manifest;

	long max_priority = -1;
	while(current_Manifest != NULL) {
		if(strcmp(current_Manifest->package->destination, destination) == 0) {
			if(current_Manifest->package->priority > max_priority) {
				max_priority = current_Manifest->package->priority;
				highest_priority_package = current_Manifest->package;
			}
		}

		current_Manifest = current_Manifest->next;
	}

	return highest_priority_package;
}

void destroy_robot(Robot* robot){
	if(!robot)
		return;

	while(robot->manifest != NULL) {
		Manifest* current_Manifest = robot->manifest;
		robot->manifest = robot->manifest->next;

		destroy_manifest_node(current_Manifest);		
		robot->size--;
	}

	free(robot);
	robot = NULL;
}

/*
*	Functie helper
*
*	Functia primeste adresa unui pointer la robot, un nume de destinatie si intoarce un pointer la un nod manifest
*			- cauta in lista robotului primul nod manifest ce contine pachetul 
*			cu destinatia primita ca parametru
*			- elimina nodul din lista robotului fara ca acesta sa fie dezalocat si reface legaturile
*           - scade size-ul robotului odata ce nodul este eliminat din lista
*			- returneaza NULL daca nu exista niciun nod ce contine pachetul cu destinatia cautata
*/
Manifest* robot_unload_package_destination(Robot** robot, const char* destination) {
	if(!(*robot) || (robot_is_empty(*robot)) || (destination == NULL))
		return NULL;

	Manifest* current_Manifest = (*robot)->manifest;
	while((current_Manifest != NULL) && 
			(strcmp(current_Manifest->package->destination, destination) != 0)) {

		current_Manifest = current_Manifest->next;
	}

	if(current_Manifest == NULL)
		return NULL;

	if(current_Manifest == (*robot)->manifest) {
		(*robot)->manifest = (*robot)->manifest->next;
		current_Manifest->next = NULL;
		(*robot)->size--;
		return current_Manifest;
	} else if(current_Manifest->next == NULL) {
		current_Manifest->prev->next = NULL;
		current_Manifest->prev = NULL;

		(*robot)->size--;
		return current_Manifest;
	}
			
	current_Manifest->prev->next = current_Manifest->next;
	current_Manifest->next->prev = current_Manifest->prev;
	current_Manifest->prev = NULL;
	current_Manifest->next = NULL;

	(*robot)->size--;
	return current_Manifest;
}

/*
*	Functie helper
*
*	Functia primeste adresa unui pointer la truck si adresa unui pointer la manifest
*			- pune nodul manifest pe ultima pozitie din lista truck-ului
*			- creste size-ul truck-ului odata ce nodul este legat in lista
*/
void truck_load_package(Truck** truck, Manifest** manifest) {
	if(!(*truck) || (truck_is_full(*truck)) || !(*manifest))
		return;

	if(truck_is_empty(*truck)) {
		(*truck)->manifest = *manifest;
		(*truck)->size++;
		return;
	}

	Manifest* truck_last_Manifest = (*truck)->manifest;
	while(truck_last_Manifest->next != NULL) {
		truck_last_Manifest = truck_last_Manifest->next;
	}

	truck_last_Manifest->next = *manifest;
	(*manifest)->prev = truck_last_Manifest;
	(*truck)->size++;
}

/*
*	Functia foloseste cele 2 functii helper de mai sus
*/
void robot_unload_packages(Truck* truck, Robot* robot){
	if((!truck) || (!robot) || (robot_is_empty(robot)) || (truck_is_full(truck)) || (!truck->destination)) 
		return;

	long i;
	long truck_free_slots = truck->capacity - truck->size;

	for(i = 0; i < truck_free_slots; i++) {
		Manifest* transfer_Manifest = robot_unload_package_destination(&robot, truck->destination);
		if(!transfer_Manifest) return;
		truck_load_package(&truck, &transfer_Manifest);
	}
}

// Attach to specific truck
int robot_attach_find_truck(Robot* robot, Parkinglot *parkinglot){
	int found_truck = 0;
	long size = 0;
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	Manifest* m_iterator = robot->manifest;


	while(m_iterator != NULL){
		while(arrived_iterator != parkinglot->arrived_trucks){
			size  = truck_destination_robots_unloading_size(arrived_iterator);
			if(strncmp(m_iterator->package->destination, arrived_iterator->destination, MAX_DESTINATION_NAME_LEN) == 0 && 
									size < (arrived_iterator->capacity-arrived_iterator->size)){
				found_truck = 1;
				break;
			}

			arrived_iterator = arrived_iterator->next;
		}

		if(found_truck)
			break;
		m_iterator = m_iterator->next;
	}

	if(found_truck == 0)
		return 0;


	Robot* prevr_iterator = NULL;
	Robot* r_iterator = arrived_iterator->unloading_robots;
	while(r_iterator != NULL){
		Package *pkg = robot_get_destination_highest_priority_package(r_iterator, m_iterator->package->destination);
		if(m_iterator->package->priority >= pkg->priority)
			break;
		prevr_iterator = r_iterator;
		r_iterator = r_iterator->next;
	}

	robot->next = r_iterator;
	if(prevr_iterator == NULL)
		arrived_iterator->unloading_robots = robot;
	else
		prevr_iterator->next = robot;

	return 1;
}

void robot_print_manifest_info(Robot* robot){
	Manifest *iterator = robot->manifest;
	while(iterator != NULL){
		printf(" R->P: %s %ld\n", iterator->package->destination, iterator->package->priority);
		iterator = iterator->next;
	}

	printf("\n");
}

Truck* create_truck(const char* destination, long capacity, 
					long transit_time, long departure_time){
	
	Truck* truck = (Truck*) malloc(sizeof(Truck));
	if(!truck)
		exit(EXIT_FAILURE);

	if(!destination) 
		truck->destination = NULL;
	else {
		truck->destination = (char*) malloc(MAX_DESTINATION_NAME_LEN * 
											sizeof(char));
		if(!truck->destination)
			exit(EXIT_FAILURE);

		strcpy(truck->destination, destination);
	}

	truck->in_transit_time = 0;
	truck->size = 0;
	truck->manifest = NULL;
	truck->unloading_robots = NULL;
	truck->next = NULL;
	truck->capacity = capacity;
	truck->transit_end_time = transit_time;
	truck->departure_time = departure_time;

	return truck;
}

int truck_is_full(Truck *truck){
	if(truck) {
		if(truck->size == truck->capacity)
			return 1;
	}

	return 0;
}

int truck_is_empty(Truck *truck){
	if(truck) {
		if(truck->size == 0 && truck->capacity != 0)
			return 1;
	}

	return 0;
}

long truck_destination_robots_unloading_size(Truck* truck){
	if((!truck) || (!truck->destination))
		return 0;

	long unloading_size = 0;
	Robot* current_robot = truck->unloading_robots;

	while(current_robot != NULL) {
		Manifest* current_robot_Manifest = current_robot->manifest;
		while(current_robot_Manifest != NULL) {
			if(strcmp(current_robot_Manifest->package->destination, truck->destination) == 0) {
				unloading_size += current_robot->size;
				break;
			}
			current_robot_Manifest = current_robot_Manifest->next;
		}
		current_robot = current_robot->next;
	}

	return unloading_size;
}


void truck_print_info(Truck* truck){
	printf("T: %s %ld %ld %ld %ld %ld\n", truck->destination, truck->size, truck->capacity,
			truck->in_transit_time, truck->transit_end_time, truck->departure_time);

	Manifest* m_iterator = truck->manifest;
	while(m_iterator != NULL){
		printf(" T->P: %s %ld\n", m_iterator->package->destination, m_iterator->package->priority);
		m_iterator = m_iterator->next;
	}

	Robot* r_iterator = truck->unloading_robots;
	while(r_iterator != NULL){
		printf(" T->R: %ld %ld\n", r_iterator->size, r_iterator->capacity);
		robot_print_manifest_info(r_iterator);
		r_iterator = r_iterator->next;
	}
}

void destroy_truck(Truck* truck){
	if(!truck)
		return;

	truck->next = NULL;
	if(truck->destination) {
		free(truck->destination);
		truck->destination = NULL;
	}

	if(!truck_is_empty(truck)) {
		while(truck->manifest != NULL) {
			Manifest* current_truck_Manifest = truck->manifest;
			truck->manifest = truck->manifest->next;	

			destroy_manifest_node(current_truck_Manifest);
			truck->size--;
		}
	}

	while(truck->unloading_robots != NULL) {
		Robot* current_robot = truck->unloading_robots;
		truck->unloading_robots = truck->unloading_robots->next;

		destroy_robot(current_robot);
	}

	free(truck);
	truck = NULL;
}

Parkinglot* create_parkinglot(void){
	// Allocate parking lot
	Parkinglot* parkinglot = (Parkinglot*) malloc(sizeof(Parkinglot));
	if(!parkinglot)
		exit(EXIT_FAILURE);

	parkinglot->arrived_trucks = (Truck*) malloc(sizeof(Truck));
	if(!parkinglot->arrived_trucks)
		exit(EXIT_FAILURE);
	parkinglot->arrived_trucks->next = parkinglot->arrived_trucks;

	parkinglot->departed_trucks = (Truck*) malloc(sizeof(Truck));
	if(!parkinglot->departed_trucks)
		exit(EXIT_FAILURE);
	parkinglot->departed_trucks->next = parkinglot->departed_trucks;

	parkinglot->pending_robots = (Robot*) malloc(sizeof(Robot));
	if(!parkinglot->pending_robots)
		exit(EXIT_FAILURE);
	parkinglot->pending_robots->next = parkinglot->pending_robots;

	parkinglot->standby_robots = (Robot*) malloc(sizeof(Robot));
	if(!parkinglot->standby_robots)
		exit(EXIT_FAILURE);
	parkinglot->standby_robots->next = parkinglot->standby_robots;

	return parkinglot;
}

Parkinglot* open_parckinglot(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Parkinglot *parkinglot = create_parkinglot();

	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		// destination, capacitym transit_time, departure_time, arrived
		if(token[0] == 'T'){
			token = strtok(NULL, ",\n ");
			char *destination = token;

			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			token = strtok(NULL, ",\n ");
			long transit_time = atol(token);

			token = strtok(NULL, ",\n ");
			long departure_time = atol(token);

			token = strtok(NULL, ",\n ");
			int arrived = atoi(token);

			Truck *truck = create_truck(destination, capacity, transit_time, departure_time);

			if(arrived)
				truck_arrived(parkinglot, truck);
			else
				truck_departed(parkinglot, truck);

		}else if(token[0] == 'R'){
			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			Robot *robot = create_robot(capacity);
			parkinglot_add_robot(parkinglot, robot);

		}

		free(line);
		line = NULL;
		len = 0;
	}
	free(line);

	fclose(fp);
	return parkinglot;

	file_open_exception:
	return NULL;
}

/*
*	Functie helper
*
*	Functia primeste adresa unui pointer la robot, adresa unui pointer la santinela unei liste si un key
*			- in functie de key, introduce ordonat robotul in lista primita 
*/
void parkinglot_add_robot_circularLinkedList(Robot** robot, Robot** dummy,
												const char* key) {
	if(!(*robot) || !(*dummy))
		return;

	if((*dummy)->next == *dummy) {
		(*dummy)->next = *robot;
		(*robot)->next = *dummy;
		return;
	}

	Robot* current_robot = (*dummy)->next;
	Robot* prev_robot = *dummy;

	if(strcmp(key, "capacity") == 0) {
		while((current_robot != *dummy) && 
			(current_robot->capacity > (*robot)->capacity)) {

			prev_robot = current_robot;
			current_robot = current_robot->next;
		}
	} else if(strcmp(key, "size") == 0)	{
		while((current_robot != *dummy) && 
			(current_robot->size > (*robot)->size)) {

			prev_robot = current_robot;
			current_robot = current_robot->next;
		}
	} else
		return;

	(*robot)->next = current_robot;
	prev_robot->next = *robot;
}

/*
*	Functia foloseste functia helper de mai sus 
*/
void parkinglot_add_robot(Parkinglot* parkinglot, Robot *robot){
	if((!parkinglot) || (!robot))
		return;

	if(robot_is_empty(robot)) 
		parkinglot_add_robot_circularLinkedList(&robot, &parkinglot->standby_robots, "capacity");
	else 
		parkinglot_add_robot_circularLinkedList(&robot, &parkinglot->pending_robots, "size");
}

/*
*	Functie helper
*
*	Functia primeste adresa unui pointer la robot si adresa unui pointer la santinela unei liste
*			- cauta robotul in lista si-l elimina,daca acesta exista, fara sa fie dezalocat
*/
void parkinglot_remove_robot_circularLinkedList(Robot** robot, Robot** dummy) {
	if(!(*robot) || !(*dummy)) return;

	Robot* current_robot = (*dummy)->next;
	Robot* prev_robot = *dummy;

	while((current_robot != *dummy) && (current_robot != *robot)) {
		prev_robot = current_robot;
		current_robot = current_robot->next;
	}

	if(current_robot == *dummy)
		return;

	prev_robot->next = current_robot->next;
	current_robot->next = NULL;
}	

/*
*	Functia foloseste functia helper de mai sus
*/
void parkinglot_remove_robot(Parkinglot *parkinglot, Robot* robot){
	if((!parkinglot) || (!robot))
		return;

	if(robot_is_empty(robot))
		parkinglot_remove_robot_circularLinkedList(&robot, &parkinglot->standby_robots);
	else
		parkinglot_remove_robot_circularLinkedList(&robot, &parkinglot->pending_robots);
}

int parckinglot_are_robots_peding(Parkinglot* parkinglot){
	if(parkinglot->pending_robots->next != parkinglot->pending_robots)
		return 1;

	return 0;
}

int parkinglot_are_arrived_trucks_empty(Parkinglot* parkinglot){
	Truck* current_truck = parkinglot->arrived_trucks->next;
	while(current_truck != parkinglot->arrived_trucks) {
		if(current_truck->size != 0) 
			return 0;
		current_truck = current_truck->next;
	}

	return 1;
}

int parkinglot_are_trucks_in_transit(Parkinglot* parkinglot){
	Truck* current_truck = parkinglot->departed_trucks->next;
	while(current_truck != parkinglot->departed_trucks) {
		if(current_truck->in_transit_time == current_truck->transit_end_time) 
			return 1;
		current_truck = current_truck->next;
	}

	return 0;
}

/*
*	Functie helper
*
*	Functia primeste adresele unor pointeri la santinela de tip Truck* sau Robot*
*			- dezaloca elementele unei liste circulare cu noduri de tip Truck sau Robot
*			- nu dezaloca santinela 
*/
void destroy_circularLinkedList(Truck** truck_dummy, Robot** robot_dummy) {
	if(robot_dummy == NULL) {
		if(*truck_dummy != NULL) {
			Truck* first_truck = (*truck_dummy)->next;
			while(first_truck != *truck_dummy) {
				Truck* current_truck = first_truck;
				first_truck = first_truck->next;
				destroy_truck(current_truck);
				current_truck = NULL;;
			}

			free(*truck_dummy);
			*truck_dummy = NULL;
			truck_dummy = NULL;
		}
	} else if(truck_dummy == NULL){
		if(*robot_dummy != NULL) {
			Robot* first_robot = (*robot_dummy)->next;
			while(first_robot != *robot_dummy) {
				Robot* current_robot = first_robot;
				first_robot = first_robot->next;
				destroy_robot(current_robot);
				current_robot = NULL;;
			}

			free(*robot_dummy);
			*robot_dummy = NULL;
			robot_dummy = NULL;
		}
	}
}

/*
*	Functia foloseste functia helper de mai sus 
*/
void destroy_parkinglot(Parkinglot* parkinglot){
	destroy_circularLinkedList(&parkinglot->arrived_trucks, NULL);
	destroy_circularLinkedList(&parkinglot->departed_trucks, NULL);
	destroy_circularLinkedList(NULL, &parkinglot->standby_robots);
	destroy_circularLinkedList(NULL, &parkinglot->pending_robots);

	free(parkinglot);
	parkinglot = NULL;
}

void parkinglot_print_arrived_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->arrived_trucks->next;
	while(iterator != parkinglot->arrived_trucks){

		truck_print_info(iterator);
		iterator = iterator->next;
	}

	printf("\n");
}

void parkinglot_print_departed_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->departed_trucks->next;
	while(iterator != parkinglot->departed_trucks){
		truck_print_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");
}

void parkinglot_print_pending_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->pending_robots->next;
	while(iterator != parkinglot->pending_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");
}

void parkinglot_print_standby_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->standby_robots->next;
	while(iterator != parkinglot->standby_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");
}

/*	
*	Functie helper
*
*	Functia primeste adresa unui pointer la truck, adresa unui pointer la santinela unei liste si un key
*			- in functie de key, introduce ordonat truck-ul in lista primita 
*/
void parkinglot_add_truck_circularLinkedList(Truck** truck, Truck** dummy, 
															const char* key) {
	if(!(*truck) || !(*dummy))
		return;

	if((*dummy)->next == *dummy) {
		(*dummy)->next = *truck;
		(*truck)->next = *dummy;
		return;
	}

	Truck* current_truck = (*dummy)->next;
	Truck* prev_truck = *dummy;

	if(strcmp(key, "departed") == 0) {
		while((current_truck != *dummy) && 
			(current_truck->departure_time < (*truck)->departure_time)) {

			prev_truck = current_truck;
			current_truck = current_truck->next;
		}
	} else if(strcmp(key, "arrived") == 0)	{
		while((current_truck != *dummy) && 
			(strcmp(current_truck->destination, (*truck)->destination) < 0 || 
				((strcmp(current_truck->destination, (*truck)->destination) == 0) && 
					(current_truck->departure_time < (*truck)->departure_time)))) {

			prev_truck = current_truck;
			current_truck = current_truck->next;
		}
	} else
		return;

	(*truck)->next = current_truck;
	prev_truck->next = *truck;
}

/*
*	Functie helper
*
*	Functia primeste adresa unui pointer la truck si adresa unui pointer la santinela unei liste
*			- cauta truck-ul in lista si-l elimina,daca acesta exista, fara sa fie dezalocat
*/
void parkinglot_remove_truck_circularLinkedList(Truck** truck, Truck** dummy) {
	if(!(*truck) || !(*dummy)) return;

	Truck* current_truck = (*dummy)->next;
	Truck* prev_truck = *dummy;

	while((current_truck != *dummy) && (current_truck != *truck)) {
		prev_truck = current_truck;
		current_truck = current_truck->next;
	}

	if(current_truck == *dummy)
		return;

	prev_truck->next = current_truck->next;
	current_truck->next = NULL;
}	

/*
*	Functia se foloseste de cele 2 functii helper de mai sus
*/
void truck_departed(Parkinglot *parkinglot, Truck* truck){
	if(parkinglot == NULL || truck == NULL) return;

	// Search through arrived list, if exists node is found remove it
	// Note: this must remove the node from the list, NOT deallocate it
	parkinglot_remove_truck_circularLinkedList(&truck, &parkinglot->arrived_trucks);
	parkinglot_add_truck_circularLinkedList(&truck, &parkinglot->departed_trucks, "departed");
}

/*
*	Functia se foloseste de cele 2 functii helper de mai sus
*/
void truck_arrived(Parkinglot *parkinglot, Truck* truck){
	if(parkinglot == NULL || truck == NULL) return;

	// Search through departed list, if exists node is found remove it
	// Note: this must remove the node not deallocate it
	parkinglot_remove_truck_circularLinkedList(&truck, &parkinglot->departed_trucks);
	parkinglot_add_truck_circularLinkedList(&truck, &parkinglot->arrived_trucks, "arrived");

	while(truck->manifest != NULL) {
		Manifest* current_Manifest = truck->manifest;
		truck->manifest = truck->manifest->next;
		destroy_manifest_node(current_Manifest);
	}

	truck->size = 0;
}

void truck_transfer_unloading_robots(Parkinglot* parkinglot, Truck* truck){
	if(!parkinglot || !truck || !truck->unloading_robots) return;

	while(truck->unloading_robots != NULL) {
		Robot*current_robot = truck->unloading_robots;
		truck->unloading_robots = truck->unloading_robots->next;

		if(robot_is_empty(current_robot)) {
			parkinglot_add_robot_circularLinkedList(&current_robot, 
							&parkinglot->standby_robots, "capacity");
		}
		else {
			parkinglot_add_robot_circularLinkedList(&current_robot, 
							&parkinglot->pending_robots, "size");
		}
	}
}

// Depends on parking_turck_departed
void truck_update_depatures(Parkinglot* parkinglot, long day_hour){
	Truck* current_truck = parkinglot->arrived_trucks->next;
	while(current_truck != parkinglot->arrived_trucks) {
		if(current_truck->departure_time == day_hour) {
			Truck* temp = current_truck;
			current_truck = current_truck->next;
			truck_transfer_unloading_robots(parkinglot, temp);
			truck_departed(parkinglot, temp);
		} else
			current_truck = current_truck->next;
	}
}

// Depends on parking_turck_arrived
void truck_update_transit_times(Parkinglot* parkinglot){
	Truck* current_truck = parkinglot->departed_trucks->next;
	while(current_truck != parkinglot->departed_trucks) {
		current_truck->in_transit_time++;

		if(current_truck->in_transit_time == current_truck->transit_end_time) {
			current_truck->in_transit_time = 0;
			Truck* temp = current_truck;
			current_truck = current_truck->next;
			truck_arrived(parkinglot, temp);
		} else
			current_truck = current_truck->next;
	}
}

void robot_swarm_collect(Wearhouse *wearhouse, Parkinglot *parkinglot){
	Robot *head_robot = parkinglot->standby_robots;
	Robot *current_robot = parkinglot->standby_robots->next;
	while(current_robot != parkinglot->standby_robots){

		// Load packages from wearhouse if possible
		if(!robot_load_packages(wearhouse, current_robot)){
			break;
		}

		// Remove robot from standby list
		Robot *aux = current_robot;
		head_robot->next = current_robot->next;
		current_robot = current_robot->next;

		// Add robot to the
		parkinglot_add_robot(parkinglot, aux);
	}
}

void robot_swarm_assign_to_trucks(Parkinglot *parkinglot){

	Robot *current_robot = parkinglot->pending_robots->next;

	while(current_robot != parkinglot->pending_robots){
		Robot* aux = current_robot;
		current_robot = current_robot->next;
		parkinglot_remove_robot(parkinglot, aux);
		int attach_succeded = robot_attach_find_truck(aux, parkinglot);
		if(!attach_succeded)
			parkinglot_add_robot(parkinglot, aux);
	}
}

void robot_swarm_deposit(Parkinglot* parkinglot){
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	while(arrived_iterator != parkinglot->arrived_trucks){
		Robot *current_robot = arrived_iterator->unloading_robots;
		while(current_robot != NULL){
			robot_unload_packages(arrived_iterator, current_robot);
			Robot *aux = current_robot;
			current_robot = current_robot->next;
			arrived_iterator->unloading_robots = current_robot;
			parkinglot_add_robot(parkinglot, aux);
		}
		arrived_iterator = arrived_iterator->next;
	}
}
