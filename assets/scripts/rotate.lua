-- Przykładowy skrypt sterujący robotem
-- Parametry: setJointRotation(numer_przegubu, kąt)

-- Obróć podstawę (przegub 0)
setJointRotation(0, 45)
wait(1)  -- Poczekaj 1 sekundę

-- Obróć pierwsze ramię (przegub 1)
setJointRotation(1, 30)
wait(1)

-- Obróć drugie ramię (przegub 2)
setJointRotation(2, -45)
wait(1)

-- Obróć nadgarstek (przegub 3)
setJointRotation(3, 90)
wait(1)

-- Wróć do pozycji początkowej
setJointRotation(0, 0)
setJointRotation(1, 0)
setJointRotation(2, 0)
setJointRotation(3, 0)