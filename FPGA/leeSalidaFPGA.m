%----------------------------------------------------------
%--------------- SISTEMAS DE TEMPO REAL -------------------
%------------------- Curso 2010/11  -----------------------
%----------------------------------------------------------
%------------ Author: Tiago M. Fernandez ------------------
%------------ Organization: GTEC, Universidade da Coruña --
%----------------------------------------------------------
%
% Parameters:
%   saidaFPGA: name of the image file to be processed
%   imaxeOrixinal: name of the original image file
%----------------------------------------------------------

function y = leeSalidaFPGA(saidaFPGA, imaxeOrixinal)

if (nargin ~= 2)
    error(' Funcion leeSalidaFPGA requiere de dos parametros: fichero de salida de la FPGA y nombre del fichero de la imagen original. \n');
end

try
    a = imread(imaxeOrixinal);

    figure;
    image(a);
    tam = size(a);

    file = transpose(uint8(textread(saidaFPGA,'%d','delimiter','\n')));
    temp = reshape(file, tam(1), tam(2), tam(3));

    figure;
    image(temp);

    y = 'Fichero procesado correctamente';
    
catch
    y = 'Error al procesar el fichero de salida de la FPGA';
    
end



