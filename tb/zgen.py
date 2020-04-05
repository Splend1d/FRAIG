import os 
dirs = os.listdir()
print(dirs)
with open("1.do.all","w") as dof:
   for d in dirs:
      if (".aag" in d) and (".out" not in d):
         dof.write("cirr "+d+' -r \n')
         dof.write("cirp -n"+'\n')
         dof.write("cirp -s"+'\n')
         dof.write("cirp -po"+'\n')
         dof.write("cirp -pi"+'\n')
         dof.write("cirp -fl"+'\n')
         dof.write("cirg 1\n")
         dof.write("cirg 2\n")
         dof.write("cirg 3\n")
         dof.write("cirg 4\n")
         dof.write("cirg 5\n")
         dof.write("cirg 6\n")
         dof.write("cirg 3 -fanin 3\n")
         dof.write("cirg 25 -fanin 4\n")
         dof.write("cirg 567 -fanin 6\n")
         dof.write("cirg 63 -fanout 5\n")
         dof.write("cirg 12 -fanin 1\n")
         dof.write("cirg 7 -fanin 1\n")
         dof.write("cirg 1 -fanout 10\n")
         dof.write("cirg 8 -fanout 1000\n")
         dof.write("cirg 10\n")
         dof.write("cirg 25 -fanin 10\n")
         dof.write("cirg 87\n")
         dof.write("cirg 1000\n")
         dof.write("cirg 1000 -fanin 1000\n")
         dof.write("cirw -o "+d+".out\n")
   dof.write("q -f\n")
         
      