# File: `seperator.h`

## Classes

- `Seperator`

## Functions

- `else if (iObeyQuotes && (message[i] == '\"' || message[i] == '\'')) {`
- `bool IsSet(int num) const {`
- `return IsSet(arg[num]);`
- `bool IsNumber(int num) const {`
- `return IsNumber(arg[num]);`
- `bool IsHexNumber(int num) const {`
- `return IsHexNumber(arg[num]);`
- `else if (i == 0 && (check[i] == '-' || check[i] == '+') && !check[i+1] == 0) {`

## Notable Comments

- /*
- */
- // This class will split up a string smartly at the div character (default is space and tab)
- // Seperator.arg[i] is a copy of the string chopped at the divs
- // Seperator.argplus[i] is a pointer to the original string so it doesnt end at the div
- // Written by Quagmire
- //			cout << i << ": 0x" << hex << (int) message[i] << dec << " " << message[i] << endl;
- // this is ok, do nothin
