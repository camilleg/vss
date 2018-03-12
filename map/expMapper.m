function em(x0,x1,y0,y1),

x=linspace(0,1,100);

if (x0<x1)
	if (y0<y1)
		y=y0+(y1-y0)/9*(10.^( (x-x0)/(x1-x0) ) -1);
	else
		y=y1+(y0-y1)*(2.^( (x-x1)/(x0-x1) ) -1);
	end
else
	if (y0<y1)
		y=y1+(y0-y1)*(2.^( (x-x0)/(x1-x0) ) -1);
	else
		y=y0+(y1-y0)*(2.^( (x-x1)/(x0-x1) ) -1);
	end
end

y=(10.^x -1)/9;
subplot(2,2,1);
plot(y);
title('Concave/Increase (x0<x1, y0<y1)');
axis off;

y=(10.^(1-x) -1)/9;
subplot(2,2,2);
plot(y);
title('Concave/Decrease (x0<x1, y0>y1)');
axis off;

y=1-(10.^(1-x) -1)/9;
subplot(2,2,3);
plot(y);
title('Convex/Increase (x0>x1, y0<y1)');
axis off;

y=1-(10.^x -1)/9;
subplot(2,2,4);
plot(y);
title('Convex/Decrease (x0>x1, y0>y1)');
axis off;
