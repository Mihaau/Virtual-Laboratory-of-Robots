-- Tablica z kątami obrotu podstawy
local angles = {45, -45, -90, 90}

-- Wykonaj kolejno obroty dla wszystkich kątów
for i, angle in ipairs(angles) do
    -- Obróć podstawę (przegub 0) o zadany kąt
    setJointRotation(0, angle)
    -- Poczekaj 1 sekundę między obrotami
    wait(1)
end

-- Wróć do pozycji początkowej
setJointRotation(0, 0)