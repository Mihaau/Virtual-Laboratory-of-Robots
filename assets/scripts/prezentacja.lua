gripObject()              -- Chwyć obiekt
wait(0.5)                 -- Poczekaj na chwyt
moveLinear(-2, 2, 0) 
releaseObject() 
moveLinear(0, 2, 2) 
-- Liczba cykli
local cycles = 3

for i = 1, cycles do
startTracing() 
moveLinear(2, 2, 0)
stopTracing() 
wait(0.5)
moveLinear(0, 2, 2)
clearTracing() 

end