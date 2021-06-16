# Summer_of_Bitcoin

Method used for solving:  
MIP(Mixed Integer programming)-  
If [x1 x2 x3 .....] is selection vector, [f1 f2 f3 .....] fee vector, [w1 w2 ....] weight vector  
We have  
x1w1 + x2w2 + x3w3 ...... <= Block Size  
if(Xj is parent of Xi) then:  
0 <=Xi <= Xj <= 1(only binary values possible)  
therefore -1 <= Xi - Xj <= 0  
similarly if Xj,Xk,Xl are parents then  
-3 <= 3*Xi -Xj -Xk -Xl <= 0  

so by using all these constraints we get cofficient matrix and bounds, now we have to maximise objective function(f1x1+f2x2......)  
which i did by solvers  
