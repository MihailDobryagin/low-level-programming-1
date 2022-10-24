BUILD_DIR=build
SRC_DIR=src

run: before build
	./Program

before:
	mkdir $(BUILD_DIR)

build: main.o file.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program
	mv $(BUILD_DIR)/Program .

main.o: $(SRC_DIR)/main.c
	gcc -c $< -o $(BUILD_DIR)/main.o

file.o: $(SRC_DIR)/db/file.c
	gcc -c $< -o $(BUILD_DIR)/file.o

clean:
	rm -r $(BUILD_DIR)/*