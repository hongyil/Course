/* 
 * CS:APP Data Lab 
 * 
 * Hongyi Liang(Andrew ID:hongyil)
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than, or equal to, the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  //De Morgan's laws,~(x&y)=(~x|~y)
  return ~(~x|~y);
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  //need to specify case x=-1
  //idea:(Tmax+1)=Tmin, but(-1+1)=0
  return (!(~(x+x+1)))&!!(x+1);
  
}
//2
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  //4 blocks for 0x12345678,each block has two numbers,each number is 4 bits binary number
  //right shift 4*2*n bits and get the last two digits
  return (x>>(n<<3))&(0xff);
}
/* 
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 2
 */
int sign(int x) {
    //32-bit right shift 31 bits
    //arithmetic right shift for 0 is 0,for 0x1xxxxxxx->0x11111111
    //the most significant digit determined the sign
    return (!!x)|(x>>31); //(0|1)|(0|-1)|
}
/* 
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) {
  //check repeated segments,first will be 32/2=16
  x=(x>>16)&x;
  x=(x>>8)&x;
  x=(x>>4)&x;
  x=(x>>2)&x;
  return x&1;
}
//3
/* 
 * bitMask - Generate a mask consisting of all 1's 
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */

int bitMask(int highbit, int lowbit) {
  int mask1=(~0<<highbit)<<1;
  int mask2=(~0<<lowbit);
  return (mask2)&(mask1^mask2);
}

/*
 * satMul2 - multiplies by 2, saturating to Tmin or Tmax if overflow
 *   Examples: satMul2(0x30000000) = 0x60000000
 *             satMul2(0x40000000) = 0x7FFFFFFF (saturate to TMax)
 *             satMul2(0x80000001) = 0x80000000 (saturate to TMin)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int satMul2(int x) {
  //first define Tmin or Tmax;
  int Tmin=1<<31;
  int Tmax=~Tmin;
  int x_sign=x>>31;
  //case 1:multiple 2
  int Mul2=x<<1;
  //if Mul2 overflow
  int overflow=(x^Mul2)>>31;
  return ((overflow & (x_sign^Tmax)) | (~overflow & Mul2));
}
/*
 * ezThreeFourths - multiplies by 3/4 rounding toward 0,
 *   Should exactly duplicate effect of C expression (x*3/4),
 *   including overflow behavior.
 *   Examples: ezThreeFourths(11) = 8
 *             ezThreeFourths(-9) = -6
 *             ezThreeFourths(1073741824) = -268435456 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int ezThreeFourths(int x) {
  //first step x*3
  //then x*3/4
  int ezThree=(x<<1)+x; //2*x+x
  int trueValue=(0x3)&(ezThree>>31);
  return (ezThree+trueValue)>>2; //divided by 4
}
//4
/* 
 * sm2tc - Convert from sign-magnitude to two's complement
 *   where the MSB is the sign bit
 *   Example: sm2tc(0x80000005) = -5.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int sm2tc(int x) {
  int sign = x>>31;
  return (x^sign)+(((1<<31)+1)&sign);
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {

	int mask1=0x55;
	int mask2=0x33;
	int mask3=0x0f;
	int mask4=(0xff)|(0xff<<16);
	int mask5=((0xff)|(0xff<<8));

	mask1=mask1|(mask1<<16);
	mask1=mask1|(mask1<<8);

	
	mask2=mask2|(mask2<<16);
	mask2=mask2|(mask2<<8);

	
	mask3=mask3|(mask3<<16);
	mask3=mask3|(mask3<<8);
	x=(x&mask1)+((x>>1)&mask1);
	x=(x&mask2)+((x>>2)&mask2);
	x=(x&mask3)+((x>>4)&mask3);
	x=(x&mask4)+((x>>8)&mask4);
	x=(x&mask5)+((x>>16)&mask5);
	return x;
}
/*
 * bitReverse - Reverse bits in a 32-bit word
 *   Examples: bitReverse(0x80000002) = 0x40000001
 *             bitReverse(0x89ABCDEF) = 0xF7D3D591
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 45
 *   Rating: 4
 */
int bitReverse(int x) {



	int mask1=0x55;
	int mask2=0x33;
	int mask3=0x0f;
	int mask4=(0xff)|(0xff<<16);
	int mask5=((0xff)|(0xff<<8));

	mask1=mask1|(mask1<<16);
	mask1=mask1|(mask1<<8);
	
	mask2=mask2|(mask2<<16);
	mask2=mask2|(mask2<<8);
	
	mask3=mask3|(mask3<<16);
	mask3=mask3|(mask3<<8);

    x=((x>>16)&mask5)+((x<<16));
    x=((x>>8)&mask4)+((x<<8)&~mask4);
    x=((x>>4)&mask3)+((x<<4)&~mask3);
    x=((x>>2)&mask2)+((x<<2)&~mask2);
    x=((x>>1)&mask1)+((x<<1)&~mask1);
    return x;
}
//float
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {

  unsigned max=0x7fffffff;
  unsigned min=0x7f800001;
  unsigned target=uf&max;

  if (target>=min){
    return uf;
  }else{
    return target;
  } 
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  //unsigned sign = uf >> 31;
  unsigned exp = (uf & 0x7f800000)>>23;
  unsigned frac =(uf & 0x7fffff);
  unsigned bias = 0x7f;
  unsigned res;
  
  // special cases: NaN and Inf
  if (exp == 0xff){
  	return 0x80000000u;
  }else if(exp<bias){
  	return (0x0);
  }else if ((exp-bias)>=31){
  	return 0x80000000u;
  }else{
  	exp=(exp-bias);
  	if (exp>22){
  		res = (frac << (exp - 23))+(1<<exp);
  	}else{
  		res = (frac >> (23 - exp))+(1<<exp);
  	}
  }

  // if sign = 1, change its sign
  if (uf>>31) {
    return -res;
  }  
  return res;
}

/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {

  unsigned exp=uf & 0x7f800000;
  unsigned sign=uf & 0x80000000;
  
  if (exp==0){
  	return (uf<<1|sign);
  }else{
  	if (exp!=0x7f800000){
  		uf=uf+0x00800000;
  	}
  	if (exp==0x7f000000){
  		uf=(uf&0xff800000);
  	}
  }
  return uf;

}

  
