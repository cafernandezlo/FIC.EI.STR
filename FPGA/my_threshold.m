function [output] = my_threshold(image, t_red, t_green, t_blue)

%% Se selecciona la parte interesante de cada componente
    aux_1 = uint8(image(:,:,1) >= t_red(1))   + uint8(image(:,:,1) <= t_red(2));
    aux_2 = uint8(image(:,:,2) >= t_green(1)) + uint8(image(:,:,2) <= t_green(2));
    aux_3 = uint8(image(:,:,3) >= t_blue(1))  + uint8(image(:,:,3) <= t_blue(2));

%% Solo valen las que coinciden

    if (true)
        % Esta es solo para debug
        aux_1 = uint8(aux_1 == 2);
        aux_2 = uint8(aux_2 == 2);
        aux_3 = uint8(aux_3 == 2);

        figure;
        subplot(131);
        imshow(aux_1*255);
        subplot(132);
        imshow(aux_2*255);
        subplot(133);
        imshow(aux_3*255);

        output = aux_1 + aux_2 + aux_3;
        output = uint8(output == 3);
    else
        % La salida normal
        output = aux_1 + aux_2 + aux_3;
        output = uint8(output == 6);
    end

end