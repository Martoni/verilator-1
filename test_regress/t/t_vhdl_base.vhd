library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity nand_nor_top is
    Port ( A1 : in  STD_LOGIC;      -- NAND gate input 1
           A2 : in  STD_LOGIC;      -- NAND gate input 2
           X1 : out  STD_LOGIC;     -- NAND gate output
           B1 : in  STD_LOGIC;      -- NOR gate input 1
           B2 : in  STD_LOGIC;      -- NOR gate input 2
           Y1 : out  STD_LOGIC;    -- NOR gate output
           Z1 : out  STD_LOGIC;    -- AND gate output
           AA1 : out  STD_LOGIC;    -- OR gate output
           AB1 : out  STD_LOGIC;    -- XOR gate output
           AC1 : out  STD_LOGIC);    -- XNOR gate output
end nand_nor_top;

architecture Behavioral of nand_nor_top is
begin
X1 <= A1 nand A2;    -- 2 input NAND gate
Y1 <= B1 nor B2;     -- 2 input NOR gate
Z1 <= B1 and B2;     -- 2 input AND gate
AA1 <= B1 or B2;     -- 2 input OR gate
AB1 <= B1 xor B2;     -- 2 input XOR gate
AC1 <= B1 xnor B2;     -- 2 input XNOR gate

end Behavioral;
