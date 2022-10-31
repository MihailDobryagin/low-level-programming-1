BUILD_DIR=build
SRC_DIR=src
TEST_DIR=test

run: before build
	./Program

before:
	mkdir $(BUILD_DIR)

build: main.o file.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program
	mv $(BUILD_DIR)/Program .
	
rebuild: clean before main.o file.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program
	mv $(BUILD_DIR)/Program .

main.o: $(SRC_DIR)/main.c
	gcc -c $< -o $(BUILD_DIR)/main.o

file.o: $(SRC_DIR)/db/file.c
	gcc -c $< -o $(BUILD_DIR)/file.o

file_test.o: $(TEST_DIR)/file_test.c
	gcc -c $< -o $(BUILD_DIR)/file_test.o

test: file.o file_test.o
	cd build; gcc -B $(BUILD_DIR) $^ -o file_test.out


clean:
	rm -r $(BUILD_DIR)