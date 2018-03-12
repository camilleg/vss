function logistic(x,r,d)

y=zeros(100,1);
y(100)=x;

while 1,

	y(1:99)=y(2:100);
	y(100) = r*y(100)*(1-y(100));
	plot(y);
	fprintf(1,'%d\t',y(100));
	pause(d/1000);
end
