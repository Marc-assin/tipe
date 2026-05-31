import os

with open("seq.twt", "r") as f:
    l = f.readlines()
    c = 0
    i = 0
    for line in l:
        seq = line.strip()
        print(i)
        i += 1
        if(os.system(rf"C:\Users\Sourangi\Documents\GitHub\tipe\BM.exe {seq}") == 0):
            c += 1
    print(c)
        
# print(os.system(rf"C:\Users\Sourangi\Documents\GitHub\tipe\BM.exe 0 3 5 6 1 2 4"))