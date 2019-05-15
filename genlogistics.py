#!/usr/bin/python3
# Used to generate logistics information for chess pieces
INVALID_SQUARE = 64
INVALID_INDEX = -1

# contifs
def fitsonboard(x):
    return x < 64 and x > -1

def tolerance(frm, to, tolerance):
    dist = abs((to % 8) - (frm % 8))
    return (dist <= tolerance) and fitsonboard(to)

# print
def printData(indices, squares):
    print("Indices:")
    print(indices)
    print("Squares:")
    print(squares)
    print("Squares Size: ", len(squares))

# generate
def genSouthNorth():
    squares = []
    north = []
    south = []
    squares.append(INVALID_SQUARE)
    for x in range(8):
        for y in range(8):
            square = x + 8 * y
            squares.append(square)

            nindex = square + y
            north.append(nindex)
            south.append(nindex + 2)

        squares.append(INVALID_SQUARE)

    print("Squares (length " + str(len(squares)) + "):")
    print(squares)
    print("South indices:")
    print(south)
    print("North indices:")
    print(north)

def genEastWest():
    squares = []
    east = []
    west = []
    squares.append(INVALID_SQUARE)
    for x in range(8):
        for y in range(8):
            square = x * 8 + y;
            squares.append(square)

            windex = square + x
            eindex = windex + 2
            west.append(windex)
            east.append(eindex)
        squares.append(INVALID_SQUARE)

    print("Squares (length " + str(len(squares)) + "):")
    print(squares)
    print("East indices:")
    print(east)
    print("West indices:")
    print(west)

def genSENW():
    squares = []
    se = [0] * 64
    nw = [0] * 64
    squares.append(INVALID_SQUARE)
    index = 0
    for col in range(8):
        square = col
        nw[col] = index
        while True:
            squares.append(square)
            index += 1
            if(fitsonboard(square - 9)):
                nw[square] = index - 1

            square += 9
            if(square % 8 == 0):
                break

            se[square - 9] = index + 1

        se[square - 9] = 0
        squares.append(INVALID_SQUARE)
        index += 1

    for row in range(7):
        square = (row + 1) * 8;
        nw[row] = index
        while(fitsonboard(square)):
            squares.append(square)
            index += 1
            if(fitsonboard(square - 9)):
                nw[square] = index - 1

            square += 9
            if(fitsonboard(square)):
                se[square - 9] = index + 1
            else:
                se[square - 9] = 0

        squares.append(INVALID_SQUARE)
        index += 1

    print("Squares (length " + str(len(squares)) + "):")
    print(squares)
    print("Southeast indices:")
    print(se)
    print("Northwest indices:")
    print(nw)

def genSWNE():
    squares = []
    sw = [0] * 64
    ne = [0] * 64
    squares.append(INVALID_SQUARE)
    index = 0
    for col in range(8):
        square = col
        ne[col] = index
        while True:
            squares.append(square)
            index += 1
            if(fitsonboard(square - 7)):
                ne[square] = index - 1

            square += 7
            if(square % 8 == 7):
                break;

            sw[square - 7] = index + 1

        sw[square - 7] = 0
        squares.append(INVALID_SQUARE)
        index += 1

    for row in range(7):
        square = (row + 1) * 8 + 7
        ne[row] = index
        while(fitsonboard(square)):
            squares.append(square)
            index += 1
            if(fitsonboard(square - 7)):
                ne[square] = index - 1

            square += 7
            if(fitsonboard(square)):
                sw[square - 7] = index + 1

        sw[square - 7] = 0
        squares.append(INVALID_SQUARE)
        index += 1

    print("Squares (length " + str(len(squares)) + "):")
    print(squares)
    print("Southwest indices:")
    print(sw)
    print("Northeast indices:")
    print(ne)

def genKnightMap():
    squares = []
    indices = []
    offsets = [-17, -10, 6, 15, -15, -6, 10, 17]
    for x in range(64):
        indices.append(len(squares))
        for offset in offsets:
            to = offset + x
            if(tolerance(x, to, 2)):
                squares.append(to)

        squares.append(INVALID_SQUARE)
    printData(indices, squares)

def genKingMap():
    squares = []
    indices = []
    offsets = [-9, -1, 7, -7, 1, 9, -8, 8]
    for x in range(64):
        indices.append(len(squares))
        for offset in offsets:
            to = offset + x
            if(tolerance(x, to, 1)):
                squares.append(to)

        squares.append(INVALID_SQUARE)
    printData(indices, squares)

# generation calls

#genNorthSouth()
#genEastWest()
#genSENW()
#genSWNE()
#genKnightMap()
genKingMap()

