# Overlook (in progress)
A scientific time series predictor for financial data. This is a master's thesis project, which will be completed eventually. The target time for completion is spring 2019.

The program is still very buggy. Please don't fork or try to run this yet. The work is currently ongoing.

### Features
Overlook uses following neural network models in some way:
- Forecasting **Autoencoder**
- Ideal order **Classifier**
- **Linear regression**
- An **Reinforcement Learning** module, based on Deep Q Learning
- **Long Short-Term Memory networks** (LSTM) 
- **Mona**, a goal-seeking neural network that learns hierarchies of cause and effect contexts
- **NARX**, Nonlinear AutoRegressive with eXogenous inputs neural network architecture

Other features available for programmers:
- Common **Neural Network modules**
- Deep **Recurrent Neural Networks** (RNN)
- **Recurrent Highway Networks** (RHN)
- **TDNN**, time delay neural network

Ideas for improvements are welcome.

Some other features:
- High performance processing via fixed size memory. Symbols, timeframes and slot-processors can only be added in the initialization.
- Bridged connection to Meta Trader 4, which allows data transfer and mql-functions in the Overlook.
- Financial event manager
- Brokerage simulator with exposure, spreads, different base currencies, etc.
- GUI for almost everything
- Cross-platform compatibility (Windows / Linux / BSDs)


### Requirements
- Ultimate++ installed with working compiling (Windows / Linux / FreeBSD)
- TheIDE package must include few other repositories from github.com/sppp: ConvNetC++, Mona, UltimateScript
- Generic formatting of symbols in the MT4 account (e.g. EURUSD, #AAPL)
- Knowledge how to debug c++ and U++ library, because there will be dragons in this path.
