import os

with open("noeuds.txt", "r") as f:
    l = f.readlines()
    c = 0
    i = 0
    for line in l:
        if not line.strip(): 
            continue
        seq = line.strip()
        print(i)
        i += 1
        print(f"\"C:\\Users\\natha\\OneDrive\\Bureau\\scripts\\C et C++\\tipe\\a.exe\" ./dossier/{i} {seq}")
        if(os.system(f"\"C:\\Users\\natha\\OneDrive\\Bureau\\scripts\\C et C++\\tipe\\a.exe\" ./dossier/{i} {seq}") == 0):
            c += 1
    print(c)

# "C:\Users\natha\OneDrive\Bureau\scripts\C et C++\tipe\a.exe" ./dossier/31 6 5 6 3 7 4 8 2 9 3 3 4 0 4 1 3 2 4 10 5 4 5 5 0 9 5 11 1 3 1 2 0 7 0 8 1 1 1 0 2 10 3 11 0 6 2 5 2 4