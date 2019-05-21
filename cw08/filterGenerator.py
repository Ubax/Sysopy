import sys


def generate(size):
    name = "filter" + str(size)
    with open(name, "w") as file:
        file.write(str(size) + "\n")
        for i in range(0, size):
            for i in range(0, size):
                file.write(str(1 / size / size))
            file.write("\n")


if len(sys.argv) >= 2:
    size = int(sys.argv[1])
    generate(size)
    sys.exit()


sizes = [3, 5, 10, 20, 30, 50, 65]
for i in sizes:
    generate(i)
