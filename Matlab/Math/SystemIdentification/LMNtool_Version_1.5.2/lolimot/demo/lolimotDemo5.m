%% lolimotDemo5 Demo 5: Dynamic process of first order with 2 inputs and 2 outputs
%
%
%   LoLiMoT - Nonlinear System Identification Toolbox
%   Torsten Fischer, 20-October-2011
%   Institute of Mechanics & Automatic Control, University of Siegen, Germany
%   Copyright (c) 2012 by Prof. Dr.-Ing. Oliver Nelles


% Initialise object
LMN= lolimot;                   % Generate an empty net and data set structure

% Create training data
u1 = randn(300,1);
u2 = sin(linspace(0,2*pi,300))';
y1 = zeros(300,1);
y2 = zeros(300,1);
for k = 2:300
    y1(k) = atan(u1(k-1)) + atan(u2(k-1)) + 0.9*y1(k-1);
    y2(k) = u1(k-1).^2 + u1(k-1)*u2(k-1) + 0.7*y1(k-1) + 0.8*y2(k-1);
end

% Assign training data
LMN.input = [u1 u2];
LMN.output = [y1 y2];

% Specify delays
LMN.xInputDelay{1} = [1]; LMN.xInputDelay{2} = [1];        % Delayed inputs
LMN.xOutputDelay{1} = [1]; LMN.xOutputDelay{2} = [1];      % Delayed outputs
LMN.zInputDelay{1} = [1]; LMN.zInputDelay{2} = [1];        % Delayed inputs
LMN.zOutputDelay{1} = [1]; LMN.zOutputDelay{2} = [1];      % Delayed outputs


LMN.maxNumberOfLM = 10;         % Termination criterion for maximal number of LLMs
LMN.minError = 0.02;            % Termination criterion for minimal error
LMN.kStepPrediction = inf;      % Simulation not one-step-ahead prediction
LMN.history.displayMode = true; % display information

% Train net
LMN = LMN.train;

% Generalization
u1G = randn(270,1);
u2G = sin(linspace(0,2*pi,length(u1G)))';
y1G = zeros(length(u1G),1);
y2G = zeros(length(u1G),1);
for k = 2:length(y1G)
    y1G(k) = atan(u1G(k-1)) + atan(u2G(k-1)) + 0.9*y1G(k-1);
    y2G(k) = u1G(k-1).^2 + u1G(k-1)*u2G(k-1) + 0.7*y1G(k-1) + 0.8*y2G(k-1);
end
uG = [u1G u2G];
yG = [y1G y2G];

% Simulate net
yGModel = calculateModelOutput(LMN, uG, yG);
JG = calcGlobalLossFunction(LMN ,yG, yGModel);

figure
subplot(1,2,1)
LMN.plotModel([],1)
subplot(1,2,2)
LMN.plotModel([],2)
