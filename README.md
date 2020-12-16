# My Shell

## Task:

Proiectati si implementati urmatorul protocol de comunicare intre procese:

- comunicarea se face prin executia de comenzi citite de la tastatura in procesul parinte si executate in procesele copil
- comenzile sunt siruri de caractere delimitate de new line
- raspunsurile sunt siruri de octeti prefixate de lungimea raspunsului
- rezultatul obtinut in urma executiei oricarei comenzi va fi afisat de procesul parinte
- protocolul minimal cuprinde comenzile:
    - "login : username" - a carei existenta este validata prin utilizarea unui fisier de configurare
    - "myfind file" - o comanda care permite gasirea unui fisier si afisarea de informatii asociate acelui fisier; informatiile vor fi de tipul: data crearii, data modificarii, dimensiunea fisierului, drepturile de access asupra fisierului etc.
    - "mystat file" - o comanda ce permite vizualizarea atributelor unui fisier
    - "quit"
- in implementarea comenzilor "myfind" si "mystat" nu se va utiliza nicio functie din familia "exec()" in vederea executiei unor comenzi de sistem ce ofera functionalitatile respective
- comunicarea intre procese se va face folosind cel putin o data fiecare din urmatoarele mecanisme ce permit comunicarea: pipe-uri interne, pipe-uri externe si socketpair


Observatii:
- termen de predare: laboratorul din saptamana 5 
- orice incercare de frauda, in functie de gravitate, va conduce la propunerea pentru exmatriculare a studentului in cauza sau la punctaj negativ

```Deadline 29.10.2020```