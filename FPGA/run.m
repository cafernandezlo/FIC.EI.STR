close all;
clear all;
clc;

image = imread('/Users/flanciskinho/Dropbox/chroma_key1.jpg');
red   = [0 110];
green = [100 255];
blue  = [0 90];

output = my_threshold(image, red, green, blue);

figure;
subplot(121);
imshow(image);
subplot(122);
imshow(output*255);

show = rgb2hsv(image);

s = size(image(:,:,2));
show(:,:,2) = zeros(s(1), s(2));

show(:,:,2) = double(show(:,:,2)) .* double(output);

figure;
imshow(hsv2rgb(show));