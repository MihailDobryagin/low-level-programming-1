BUILD_DIR=build
SRC_DIR=src

run: build
	$(BUILD_DIR)/Program

build: main.o file.o
	cd build; gcc -B $(BUILD_DIR) $^ -o Program

main.o: $(SRC_DIR)/main.c
	gcc -c $< -o $(BUILD_DIR)/main.o

file.o: $(SRC_DIR)/db/file.c
	gcc -c $< -o $(BUILD_DIR)/file.o

clean:
	rm -r $(BUILD_DIR)/*