BUILD_DIR=build
SRC_DIR=src
TEST_DIR=test

run: before build
	./Program

before:
	mkdir $(BUILD_DIR)

build: main.o file.o db.o manage.o entities.o strings.o std_out.o test_objects_creation.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program
	mv $(BUILD_DIR)/Program .
	
rebuild: clean build

main.o: $(SRC_DIR)/main.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/main.o

entities.o: $(SRC_DIR)/db/entities.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/entities.o

file.o: $(SRC_DIR)/db/file.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/file.o
	
db.o: $(SRC_DIR)/db/db.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/db.o

manage.o: $(SRC_DIR)/client/manage.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/manage.o

strings.o: $(SRC_DIR)/utils/strings.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/strings.o
	
std_out.o: $(SRC_DIR)/utils/std_out.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/std_out.o

test_objects_creation.o: $(SRC_DIR)/test_utils/test_objects_creation.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/test_objects_creation.o

file_test.o: $(TEST_DIR)/file_test.c
	gcc -Wall -g -c $< -o $(BUILD_DIR)/file_test.o

test: file.o file_test.o
	cd build; gcc -Wall -g -B $(BUILD_DIR) $^ -o file_test.out


clean:
	rm $(BUILD_DIR)/*
	rm Program
