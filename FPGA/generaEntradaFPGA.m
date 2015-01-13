%----------------------------------------------------------
%--------------- SISTEMAS DE TEMPO REAL -------------------
%------------------- Curso 2010/11  -----------------------
%----------------------------------------------------------
%------------ Author: Tiago M. Fernandez ------------------
%------------ Organization: GTEC, Universidade da Coruña --
%----------------------------------------------------------
%
% Parameters:
%   imaxe: name of the image file to be processed
%   ficheiroSaida: name of the file that will be writen
%
%----------------------------------------------------------

function y = generaEntradaFPGA(imaxe, ficheiroSaida)

if (nargin ~= 2)
    error(' Funcion generaEntradaFPGA requiere de dos parametros: imagen de entrada y nombre del fichero de salida. \n');
end


try
    a = imread(imaxe);


    figure(1);
    image(a);


    tam = size(a);
    long = tam(1) * tam(2) * tam(3);

    b = [reshape(a, 1, long)];


    fid = fopen(ficheiroSaida,'w');
    for i=1:length(b)
        fprintf(fid,'%d ',b(i));
        fprintf(fid,'\r\n');
    end
    fclose(fid);

    y = 'Fichero de entrada a la FPGA generado correctamente';
catch
    y = 'Error al procesar imagen de entrada o al escribir el fichero de salida';
end
