BUILD_DIR=build
SRC_DIR=src
TEST_DIR=test

run: before build
	./Program

before:
	mkdir $(BUILD_DIR)

build: main.o file.o db.o manage.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program
	mv $(BUILD_DIR)/Program .
	
rebuild: clean build

main.o: $(SRC_DIR)/main.c
	gcc -g -c $< -o $(BUILD_DIR)/main.o

file.o: $(SRC_DIR)/db/file.c
	gcc -g -c $< -o $(BUILD_DIR)/file.o
	
db.o: $(SRC_DIR)/db/db.c
	gcc -g -c $< -o $(BUILD_DIR)/db.o

manage.o: $(SRC_DIR)/client/manage.c
	gcc -g -c $< -o $(BUILD_DIR)/manage.o

file_test.o: $(TEST_DIR)/file_test.c
	gcc -g -c $< -o $(BUILD_DIR)/file_test.o

test: file.o file_test.o
	cd build; gcc -g -B $(BUILD_DIR) $^ -o file_test.out


clean:
	rm $(BUILD_DIR)/*
	rm Program
