# Overlook

Overlook is a scientific-ish time series predictor for market data.

Overlook is using neural network code from academic research projects:
 - ConvNetJS, a popular convolutive neural networks library, via ConvNetC++
 - NARX from NarxSim master's thesis project
 - Mona from Cortical Networks (MICrONS) Workshop sponsored by IARPA

Overlook uses in some way following neural network models:
- Common **Neural Network modules** (fully connected layers, non-linearities), Classification (SVM/Softmax) and Regression (L2)
- An **Reinforcement Learning** module, based on Deep Q Learning
- Deep **Recurrent Neural Networks** (RNN) 
- **Long Short-Term Memory networks** (LSTM) 
- **Recurrent Highway Networks** (RHN)
- **Mona**, a goal-seeking neural network that learns hierarchies of cause and effect contexts
- **NARX**, Nonlinear AutoRegressive with eXogenous inputs neural network architecture
- **TDNN**, time delay neural network

If some most advanced neural network is not included, please send a message to the author.


Some other features:
- High performance processing via fixed size memory. Symbols, timeframes and slot-processors can only be added in the initialization.
- Bridged connection to Meta Trader 4. It allows data transfer and mql-functions in the Overlook.
- Financial event manager
- Brokerage simulator with exposure, spreads, different base currencies, etc.
- Graphical GUI for almost everything
- Cross-platform compatibility (Windows / Linux / BSDs)

This is basically everything you can ask for from a MT4 expert advisor, unless you are asking stupid questions.
