topic "Journal";
[#phR $$0,0#00000000000000000000000000000000:Default]
[=phN1;*cR+150 $$1,1#26C05E505BFA12B5D94A958416AE5FF3:Chapter 1]
[=phN11;*R+117 $$2,2#F98403FAA9BE0040ACA893F21E4D9F2D:Chapter 2]
[=phN111;*R $$3,3#0680C0FF67EEE4E167183D1E74BF079F:Chapter 3]
[{_}%FI-FI 
[s1; Journal&]
[s0; This journal exist to encourage other programmers to write the 
progress in this way. It is a voice of a casual narrator about 
the progress of the Overlook. Please note that my native language 
is not English, but Finnish. This journal is intended for the 
audience who see some entertainment value in this. This shouldn`'t 
be considered to be final statement about anything.&]
[s0;= &]
[s2; Pre`-Release Journal&]
[s0; This is the time before the first working version. It is about 
making small single modules working, and then combining them 
to a bigger working module, until a one working program is achieved.&]
[s3; 26.4.2017&]
[s0; Welcome to the first entry of this journal.&]
[s0; Today, I am trying to get runner working. Slots should be processed 
in very varying and complex order. Slots have dependencies, and 
first thing is to make the graph to draw them in a nice way. 
The time is 10.20 and there is snow raining outside my window. 
The sky is completely white from clouds. The snow what was melted 
during early spring is replaced again by new snow. The temperature 
is around 0 Celcius. I am way behind schedule if I check how 
much coffee I have been drinking already. This is a isolated 
place with long, dark and cold winter and short summer, which 
is the very essential reason for my programming. I would rather 
travel to Greece for most of the winter.&]
[s0; Graph works. I noticed missed links from that image. Next: the 
processing batches, which can have concurrent internal processing. 
The progressing is kind of brute`-forcing now. I need to get 
this finished by end of may. Luckily, I have free time now from 
the university where I am studying computer science.&]
[s0; Ok, so I planned the new batched runner thing. I checked the 
huge todo and problem list, and then checked what code exists 
and what should be added. That`'s the normal method. I can`'t 
start implementing immediately, because it requires a more practical 
mindset. I must take a break before it. I`'m listening to classic 
rock internet radio channel... now playing Pearl Jam `- Even 
Flow. I usually listen to classic rock or ambient electronic 
internet radio. I can`'t stand too repeating playlist.&]
[s0; I got the batches formed. The groups are interesting. There 
is 8 groups in total currently. Such a difficult time to progress 
now. Planned stuff doesn`'t fit well, and some old requirements 
are invalid.&]
[s0; Finished. The RunnerCtrl now has batches. The result is a lot 
different what I planned, but it turned out well. I am too tired 
to debug it, though... This was rather difficult day, mostly 
because I had to re`-plan the implementation few times and then 
again change it while implementing. The change of the plan is 
the worst energy consumer... Usually I don`'t have to re`-plan 
this much.&]
[s3; 27.4.2017&]
[s0; The first batch is being processed. It happens somewhat correctly. 
Seems like locking data areas for processing is a mess, and it 
must be solved next. I am thinking about adding few more arguments 
to `"AddDependency`" and then locking data areas for them, and 
also asserting that data is really locked already at use time. 
Without that, multithreading seems to be slower than single thread 
:D ...that`'s bad.&]
[s0; Also, I was thinking, that some continuous training would be 
great. Agent slots doesn`'t need to be necessarily entirely trained 
and processed, but all of them can be done incrementally. As 
the input stays same, all neural networks just fits better to 
existing and they doesn`'t change much. For that reason it is 
possible. Basically, the whole stack will be trained instead 
of just going from bottom to top once. That, while reacting fast 
to the real`-time events simultaneously, will be a challenge.&]
[s3; 28.4.2017&]
[s0; Last evening I got an idea, that the nn`-slot`-stack should 
have specific time how long it can be trained. In that way, uniform 
training could be guaranteed. Ever symbol/timeframe/custom`-slot 
would get it`'s own share of processing time. The task`-switch 
of slot`-processing is very light procedure, so rather fast switching 
is possible. Only at least one full processing cycle is required 
before moving to next batch, and lower slots should be ahead 
of higher in processing position, but that is only recommended, 
not required.&]
[s0; Hmm... ok, so I have been reducing those spinlocks and memory 
releases from old model, and I think that the current small memory 
model is too fine`-grained for efficient usage. All time`-slots 
for symbol are single memory units, and that causes too much 
overhead. Previously I did the normal thing: reserved one long 
vector for sym/tf/slot, but it had other problems. I guess, I 
must fall back into that. There was a possibility, that slots 
would require same type from different sym/tf, which required 
preparing for sym/tf complexities inside one type of slot, but 
it never came true. Now all reservations for those complexities 
can be pruned away. I must say, that the current API was developed 
with this small memory model and that is better than previous, 
but the underlying model must be returned to have whole time`-range 
in one vector. Actually, the small memory model was extremely 
compact, and I could set the memory limit to 640kb, with over 
1GB cache, and it would still work :) ...but now that feature 
causes only overhead.&]
[s0; Ok, so the normal memory model raises the minimum requirement 
from that 640kb to a lot more. My PC and server can have more 
than 64GB SSD swap memory, so it won`'t be problem for me, but 
we try to think little guys too. My PC is by the way AMD CPU 
8`-core 4GHz, Memory 8GB DDR3, and I don`'t care graphics any 
more. I guess it`'s still high`-end PC currently. I have always 
had AMD cpu in my built`-from`-parts PC since Amiga 500.&]
[s0; Looks like there is some Fyre Festival situation going on. Some 
critique is not valid, since there is clearly images about the 
accomodation being in that kind of tents. However, the marketing 
is too rosy as always and common sense was missing. I was the 
opposite of the rich kid, and while I never would have fallen 
into buying a Fyre ticket, that event could have also been a 
success, if that huge ticket money was spent properly. Unfortunately, 
the high price was a scam, like too many times in the past. Next 
time when someone tries something similar, they should have a 
great 24/7 buffet and the real security.&]
[s0; Seems like changing the memory model is a big operation. I had 
to comment the existing code out, and it`'s hard to see what 
needs to be done. It will sort out eventually.&]
[s0; Didn`'t finish the memory model changing today.&]
[s3; 29.4.2017&]
[s0; The program is now running and it seems to be free of memory`-errors. 
I could valgrind it, but it probably shows clean, because the 
U`+`+ memory debugger usually catches worst problems. It starts 
and stops cleanly, but slowly. Some kind of beautiful waiting 
screen is needed.&]
[s0; Today, the target is to get first two batches processed, which 
is rather easy one.&]
[s0; The first batch done. The second batch crashes to some asserts. 
How typical.&]
[s0; The second batch done, earlier than expected.&]
[s0; When I have time to work all day, then the second part of the 
day is usually full of mistakes and difficulties to progress 
because of me being tired. I overcome that by solving simpler 
problems and fixing bugs only at next morning. It`'s usually 
comfortable enough, but it requires knowing that you can`'t do 
as good as in the morning and you shouldn`'t even try. The most 
stressful thing is to have deadlines and strict jobs while being 
tired, and being stressed is bad in the long run. Writing natural 
language notes, like this, is also useful, because you can collapse 
your thoughts into something concrete. It takes time to get into 
this kind of work routine and many kind of rewards must be involved.&]
[s0; I added serialization functions for remaining classes. I`'m 
not sure if they work at first try. There is some tricky reloadings.&]
[s3; 6.5.2017&]
[s0; One week since previous entry. I didn`'t have much time to do 
this at this week and I forgot this journal. I feel that this 
is a little bit silly. I am going to give it a try anyway.&]
[s0; This Overlook repository was forked once during the week, and 
I got nice message concerning improvements of the program. This 
is a very little amount of activity compared to popular open 
source projects, but big deal for me. Forking this is still discourged, 
as the app doesn`'t have any useful and working state yet.&]
[s0; During the week, I fixed the processing of recurrent neural 
network slots. The values might be still incorrect, but I can`'t 
fix them yet. I can diagnose the data, only when the app runs 
completely. So, one step at a time.&]
[s0; This is absolutely massive project for one programmer. I would 
highly appreciate other contributors. Also, processing times 
are worrisome for the current computing capacity. I am not sure 
that is the neural network training time something acceptable 
and sane yet. The training must be repeated eventually, and probably 
sooner than later, so I don`'t want to wait two weeks after reset...&]
[s0; I also created a proceeding document in the [/ /docs] folder. 
I am doing the master`'s thesis later, and that might be re`-used 
in the thesis. It also helps a lot to keep this project coherent. 
In that paper, you must say things as clear as possible, and 
test results must be handled properly, which is very helpful, 
even if the paper isn`'t published ever. I am still looking for 
to finish this project in this May 2017, but the paper will propably 
remain unfinished for some time.&]
[s0; Today`'s objective is to get all agents running, but if something 
bigger must be implemented, then it won`'t happen. After that 
the big loop should run without problems, but the values will 
still need bugfixes.&]
[s0; Overlook is basically huge pipeline. It has complex dependencies, 
which causes processing to be done in batches, which is like 
car assembly line, where different stuff is done in different 
places. To progress with the implementation efficiently, programmer 
have to use a technique, which has different kind of rounds. 
These rounds can take from hours to weeks. For example, round 
structure has been following so far (`+ what I assume happening 
next):&]
[s0;i150;O9; 1. Planning of the pipeline&]
[s0;i150;O9; 2. Creating all classes and main functions, and implementing 
main frame, including threading model and all busy loops. The 
program must shutdown properly, without memory leaks etc. , in 
this point. The pipeline should work with dummy classes after 
this point.&]
[s0;i150;O9; 3. Implementing all classes and adding them to the pipeline. 
Code must compile but not run. The programmer must be experienced 
to be able to program without testing the code.&]
[s0;i150;O9; 4. Running the program and fixing the runtime bugs in 
the pipeline.&]
[s0;i150;O9; 5. Implementing and testing rest of the GUI features. 
This usually requires so much fast iterative testing and fixing, 
that it must be done in per`-class basis. The pipeline must be 
able to be restored from the cache file in 20 seconds for this 
method to work.&]
[s0;i150;O9; 6. Analysing the pipeline and checking all values with 
the nice new GUI. It must be constant and everything must be 
logged in a fail`-safe way.&]
[s0;i150;O9; 7. Polishing the turd. Release the little fucker into 
the wild and see what happens. In this point, you really must 
clean the code and do or discard your TODO list.&]
[s0; I am currently at the round 4 and I try to finish it today. 
This requries much patience from the programmer and this can`'t 
be recommended to noobs.&]
[s0; Aaaand round 4 finished. I have still time to: test multithreading, 
fix re`-processing looping broker reset, the change slot has 
[/ cost] everywehere but shouldn`'t.&]
[s3; 7.5.2017&]
[s0; The analyzer part needs planning currently. The paper what I`'m 
writing requries that, and it`'s also useful in general.&]
[s0; Oh, I needed the ideal orders slot, and then I thought how it 
could be used in forecasting or with agents, and then I remembered 
two types what I ignored previously.&]
[s0; The ignored forecaster was autoencoder neural network, which 
basically has very tight bottleneck in the center, which causes 
very generalized output, which is good if you want very generic 
forecast. That might be useful input in the combining Forecaster 
class. Alone it won`'t work well.&]
[s0; The ignored agent was classifier neural network. I didn`'t know 
how to imlement this previously. Basically, it uses that ideal 
order signal as output, which is `"the shortest route to highest 
value accumulation`". It is unrealistic to expect finding that 
in real time, but it is possible afterwards. The market behaves 
very randomly, so the classifier won`'t perform well alone, but 
it can be a good additional signal for meta`-agent.&]
[s3; 9.5.2017&]
[s0; A lot of plan changes today. I needed my old A`* search algorithm 
code. When I found it, I decided to import some other old code 
too. You can check it from CoreUtils... Anyway, one thing led 
to another, and I need to redesign everything from the AnalyzerCtrl 
viewpoint. Ok, not everything, but many things. There was the 
QueryTable code and I noticed that it was also a very simple 
predictor and there is always demand for different types of predictors 
in this project. I noticed, that the QueryTable is so generic, 
that it could be used as a dummy alternative instead of almost 
any forecaster or agent. The QueryTable imports integer values 
and exports integer values, if you reduce the string`-complexity. 
Comparing QueryTable and NN forecasters is kind of like comparing 
discrete and continuous probability distributions.&]
[s0; So, all NN classes in Overlook could be over`-simplified to 
QueryTables, and all values could be changed to be discrete values 
in some range. That might be stupid to implement, but for me 
it gave a new idea about class structure for connecting classes 
to each other, and by classifing different types of signals.&]
[s0; I haven`'t developed this idea yet, but seems like from analyzer`'s 
viewpoint, you should make the testing more comprehensive and 
automatic, and the combination of slots should be done based 
of that testing [_ instead of the manual loader.]&]
[s0; Currently user sets what slots he wants to use, but the premise 
is that user wants the best performance and better solution would 
be that the performance analyzer combines best performing slots 
or uses existing pre`-analyzed combination.&]
[s0; One expects, that NN forecasters and agents outperforms QueryTable 
alternatives, but it should be measured.&]
[s0; This also opens possibility of out`-of`-order slot connections. 
Basically it causes falling back to one class object per symbol/timeframe. 
It might be too complicated, though. Also, there is no need to 
make unreasonable combinations and the rules for combinations 
will still be rather strict.&]
[s0; [* Damn, so much more work.]&]
[s0; I guess, that the basis will be some kind of prioritising system, 
which can connect different kind of slots in different symbol/timeframes. 
It prunes out probably known outcomes and verifies the best performing 
combinations.&]
[s0; The prioritising system needs a new Slot`-class replacement 
and that new class should prefer smallest possible processing 
unit (1. sym`+tf, 2. sym, 3. tf, 4. all). [_ All kind of processing 
units should still be possible,] because that is required by 
some slots (e.g. NARX). Different phases could be: source`-data, 
indicator, forecaster, forecaster`-combiner, agent and agent`-combiner. 
Values between slots are classified based on their origin`-phase 
and type of value. Custom slots will define what classes of values 
they can use. It is better in that way instead of defining exact 
sources of values. Some slots could export same signal in floating`-point 
values and in integer`-values.&]
[s0; The overview of the change of the plan is now clear, but details 
are still missing. This is almost enough from today. Must sleep 
on this. FML.&]
[s3; 10.5.2017&]
[s0; I have been thinking how to implement new changes with least 
work and working time. Seems like I should build another base 
system from scratch and then copy`-paste all compatible parts. 
I could skip the reference memory model, because this can be 
done better without it. That means skipping all [/ linking] of 
paths. It was essential in the research phase, but it is not 
required for final product anymore. It was flexible for testing 
and for searching the final plan.&]
[s0; Another shortcut is that I could combine [/ Core] and [/ Ctrl ]code. 
That would mean that e.g. MovingAverage does both drawing the 
graph and calculating values. It`'s far from [/ high performance 
computing] as we know it, but I am going to run this in desktop 
PCs anyway. I am not huge fan for simplifying the program that 
far.&]
[s0; I am going to make all changes in place of current project files. 
I will put all files in the Overlook folder this time.&]
[s0; Now, let`'s do the planning. Custom classes and their features 
will be same, but the underlying system changes, and that must 
be planned:&]
[s0;i150;O2; All graphics and computing are in the same classes&]
[s0;i150;O2; Different phases in the pipeline are: source`-data, indicator, 
forecaster, forecaster`-combiner, agent and agent`-combiner.&]
[s0;i150;O2; Prioritizer has huge sym/tf/phase space to prioritize 
different combinations of pipeline`-units. The success of one 
combination will prioritize it higher on all sym/tf, etc.&]
[s0;i150;O2; Prioritizer`-slots are tied to phase and their import 
and export values have typed values (by their phase and type 
of signal). I guess these rules limits combinations enough.&]
[s0;i150;O2; Prioritizer`-slots define their processing area size. 
They can process 1. sym`+tf, 2. sym, 3. tf, 4. all. My traditional 
indicator collection in archives can be imported when sym`+tf 
are supported again.&]
[s0;i150;O2; The processing queue in the Prioritizer is derived from 
current Session class. The prioritized queue will replace batches, 
and analyze`-queue will change priorities when processing have 
been completed. Basically all [/ processing`-receipes] exists in 
the same queue and only the processing will trigger the creation 
of the processing slot object. All priority changes are done 
after the analyzing, which can be followed from the GUI.&]
[s0;i150;O2; The paper need results, which can be exported from analyzing 
results. It can be improved easily over time, when all the primitive 
analyzing is done in small parts at first, and the more comprehensive 
analyzing can be done from same points later.&]
[s0; I am little sad about the need to rewrite so much. This version 
required some seriously hard work already. However, this is 12th 
time during 6 years when this kind of rewrite plan had to be 
made, if I remember correctly. I wrote the progress down somewhere, 
but I have no clue where that text is.&]
[s0; The QueryTable will be the first dummy slot in every phase. 
Even the prioritizing could be done with it. Then NN archs can 
compete with it, and when they outperform QueryTable, they can 
the primary processor.&]
[s0; Round 2. Creating all classes. Also, moved everything to sourceforge 
in part of the new UltimateScript project :)&]
[s0; ...&]
[s0; Well this was unexpected day. I copied all the most important 
existing code and tried to reduce useless complexity at the same 
time. The line count is around 10000 lines, which is good.&]
[s3; 11.5.2017&]
[s0; The implementation plan for Prioritizer class was getting more 
clear today. Instead of iterating all combinations of slots somehow, 
the combination must be created from last to first based on QueryTable 
values. For most of the slots, there is huge amount of input 
switches, which suggests that combination iteration is very bad 
solution.&]
[s0; I also planned the featuers of QueryTable custom slots. They 
are very simple when compared to neural network alternatives, 
but that is good at the beginnig as NN slots can be added later.&]
[s0; Today`'s tasks are:&]
[s0;i150;O0; implementing queue generating function using QueryTable, 
which requires slot`-value`-types etc.&]
[s0;i150;O0; continue implementing QueryTable classes until time 
runs out&]
[s0; I might not complete the first task today. It`'s very complex.&]
[s0; ...&]
[s0; I searched the import and export types of values for existing 
classes. This changes plan again. These types can automatically 
be connected with QueryTable and existing custom slots can be 
provided as bridges between value`-types. So these bridges between 
value`-types are being connected, and the combinations are tested. 
&]
[s0; Seems like this should be started from new project. The existing 
package is a mess. I will add good stuff from it later. Sadly, 
this journal stays here too.&]
[s0; ...&]
[s0; This priority`-queue maker from recursive reverse visiter is 
a tough nut to crack. I haven`'t had this difficult problem in 
months. &]
[s0; ...&]
[s0; Finally, I cracked that nut. The combination of slots is a binary`-string. 
It enables some features which I am too tired to describe now. 
One thing sure is that I will start implementing it tomorrow. 
It allows very fast QueryTable processing and small memory footprint.&]
[s0; Basically, you can put all arguments affecting the system into 
the binary`-string, and you can easily fetch all results relating 
to a single argument. Then you can analyze how valuable some 
argument`-value is. In the end the application can show detailed 
list of reasons why the best combination is what it is.&]
[s0; This was exhausting day, and I almost didn`'t write code at 
all. Very weird... I just refined this plan. The complete opposite 
is to do some hand routine where you don`'t need to think at 
all.&]
[s0; [_ This day will be the cornerstone of the results of my master`'s 
thesis, however.]&]
[s3; 12.5.2017&]
[s0; Yesterday was especially troublesome day. It is almost unbelieveable 
that I managed to summarise the final binary`-string plan. The 
clues kept coming throughout the day, but they weren`'t obvious.&]
[s0; Seems like the project progresses normally today. I`'m on the 
way of implemention already. The tasks for today are:&]
[s0;i150;O0; custom slots must export input `& output types&]
[s0;i150;O0; in `& out types must be collected at custom slot registration&]
[s0;i150;O0; RefreshQueue combination queue maker&]
[s0;i150;O0; unique slot processing queue for combination queue&]
[s0;i150;O0; new slot objects to the queue objects or move from previous 
queue&]
[s0;i150;O0; the correct processing order based on dependency relationships&]
[s0;i150;O0; processing of slots&]
[s0; Yet, yesterday I was completely stuck with the plan.&]
[s0; ...&]
[s0; This comination`-queue maker is the worst bottleneck. I made 
a plan and schematic`-like picture, but I still almost get stuck. 
This is completely new topic for me, maybe that`'s why.&]
[s0; ...&]
[s0; The progressing is exceptionally slow. The RefreshQueue combination 
queue maker has such a resistance. &]
[s0; I managed to iterate all initial combinations, but some of them 
might be bad. There are some rules for those, and I haven`'t 
checked them all yet.&]
[s0; Ok, the progressing speed was 1/4 of what I hoped, but that 
is still better than being stuck. This was almost fun and it 
was pure entertainment compared to the confused planning of yesterday.&]
[s3; 13.5.2017&]
[s0; I found a nice graphical list of neural network architectures 
via stumbleupon http://www.asimovinstitute.org/neural`-network`-zoo/ 
. It would be interesting to implement all of these in the Overlook 
and make a performance comparison. The included ConvNetC`+`+ 
supports almost all of those already, and many of those has been 
demonstrated in the ConvNetC`+`+ examples. The missing features 
are rather easy to implement with the correct documentation. 
I don`'t have high hopes for finding anything useful from those, 
but it sure would be interesting. It would also be good for the 
sake of completeness.&]
[s0; Today things go smoothly. The speed of progress has been improving 
two days in a row.&]
[s0; There is some epic computer security problem going on. Every 
news source reports it. Those doesn`'t really affect my FBSD 
systems. This is vulnerable, of course, but the reward for exploiting 
them is so low, that probably no one bothers. The feeling of 
open and secure OS is just best.&]
[s0; ...&]
[s0; I did some major restructuring. The new Factory class handles 
all custom classes in the project now. The main overlook program 
is just opening static objects which are registered to the factory. 
Very simple and nice.&]
[s0; ...&]
[s0; Seems like in 24 hours happened `"Biggest Ransomware Attack 
In History`". Only news reports about it yet. Some smaller blogs 
too. ZeroHedge has long discussion topic about it, and . The 
New York Times has `"Animated Map of How Tens of Thousands of 
Computers Were Infected With Ransomware`" (https://t.co/LDFCW9K5Ls) 
which is interesting, because it shows a global response in one 
hour. This may not be big thing in total cost of damages, but 
it will cause huge wave of discussion in every smallest possible 
traditional news source, [_ and at the same time globally.] That 
will be a wake up call about the real global scale of human establishment 
for many for sure. Unfortunately, it will cause an opposition 
of people wanting to ban bitcoin and crypto`-currencies, without 
understanding that they want to ban a mathematical algorithm, 
which was invented a short time ago.&]
[s0; ...&]
[s0; I managed to implement all the queueing today. Still, querytable 
isn`'t being used. Seems like I must postpone it until result 
vector has enought items in it.&]
[s3; 14.5.2017&]
[s0; Heavy snowfall at outside. Usually this doesn`'t happen in May. 
I haven`'t seen anything like this ever before. This feels like 
January and I can`'t tell any difference from the view what I 
see from my window. It should be almost summer. Two years ago 
the may was especially sunny and between 15`-20 degress of Celcius 
at daytime, now it rains snow all the time. This is still somewhat 
normal in Finland, but not usual. This year`'s potato farming 
will definitely suffer with this trend.&]
[s0; I am implementing the processing of the job`-queue, but there 
is some dragons ahead. Previously I tested some high`-performance 
computing model, but it had to be ditched eventually. It just 
wasn`'t flexible enough and benefits were also lost in the end. 
Now I am falling back to the model, which is very similar to 
the MetaTrader`'s MQL language. This, however, allows processing 
of multiple symbols and timeframes at once, which is requirement 
for classes like NARX. There is some positive implications from 
this too: I can import all my old indicators to the overlook. 
If it had been any more difficult than copy`-pasting, I would 
have skipped it, but now I can adjust the base class to do just 
that.&]
[s3; 15.5.2017&]
[s0; I am continuing developing Overlook in github. I tested to combine 
all libraries to one repository, but it wasn`'t such a good idea. 
First of all, the UltimateScript, where I added them, could have 
them in a repository, but they should be converted to it`'s own 
GUI`-libraries, which hasn`'t been started even yet, in it takes 
forever to get there. Second, I can`'t showcase invidual modules, 
which has unique value. I am going to work on UltimateScript 
this summer, though, but it`'s better to start that from empty 
desk (or repository).&]
[s3; 17.5.2017&]
[s0; Implementing the Prioritizer class was a lot more difficult 
than expected. Yesterday, I had to add features which caused 
more complexity to it again. Today I finally got into debugging 
it. Now I am creating functions to dump all the debugging information, 
and even that is rather difficult. I am not complaining about 
it, but just writing it as a note. You can`'t always do easy 
routines, which can be done as fast as your hands can write. 
It would be a bad sign if you could.&]
[s0; ...&]
[s0; Looks like the initial job`-queue is being formed correctly. 
It was difficult to debug the prioritizer enough to do that. 
I am afraid that there will be some hard`-to`-find bugs left 
there. Not good. Now I could finally start work on basic graph 
indicators and make them work. Almost enough from today... must 
have a break, at least.&]
[s3; 18.5.2017&]
[s0; Today, I lost my focus because of some troubles around me. In 
the end, I managed to restore my good spirit by thinking symbolically 
this Overlook project and what it reminds me of. I wrote very 
little code, but I found some new thoughts what I would like 
to share in this. I am just improvising here...&]
[s0; Overlook is, essentially, about watching the most important 
aspects of the whole human establishment on Earth. So, what if 
you take away humans from the equation, and think it as one single 
super`-organism, which could work even without humans or animals. 
You would get a huge lifeless or plant`-like organism, where 
evolution of technology etc. would happen virtually via databases 
and mechanical plants. There would a enormous single database 
of everything.&]
[s0; What if we would re`-introduce entities with neural`-networks 
into that enormous almost lifeless plant`-like organism? Micro`-organisms 
with brains, like birds and fishes, but not quite same. Something 
completely new. The super`-organism would create their body and 
give a spark for thinking, but what would happen next? Technically, 
their brain would start to train the huge database and to recognize 
the image of the structure of the super`-organism. Symbolically, 
they would duplicate a tiny portion of perfect information into 
the weight of their neural networks. While training their brains, 
they would start to feel enlightened. Perplexity of the training 
would go lower, and they would feel really, really small when 
they compare their training`-data to all possible information.&]
[s0; You and me feel some constant signals, like background noise, 
pain in the butt while sitting, and the temperature of the room. 
Those neural`-network entities would also have some of those. 
Have you ever suddenly remembered, that you have some event next 
day, which you had completely forgotten? Those entities would 
have same senses. Other example is, that sometimes the lecturer 
will give you new inspiring thoughts, and sometimes you are waiting 
the class to end and the lecturing is just background noise. 
You and me would probably prefer that curious feeling of learning 
something new and important, instead of making the time go by 
as fast as it can. My guess is, that the curious feeling is technically 
just very fast forward`- and backward`-propagation in biological 
synapses with lowering perplexity of the training data, which 
is connected as a reward to some hormone producer. That is a 
over`-simplification, of course.&]
[s0; The conciousness of those neural`-network entities would be 
the increasing harmony with the perfect information, the information 
by itself and their biological structure being produced by the 
super`-structure for whatever reason. The probability of their 
existence would be lower, than existence of the super`-structure 
and the information. So, one part of the conciousness would also 
be the understanding, that they really do exist despite improbabilities.&]
[s0; This is just me goofing around. Don`'t take it too seriously... 
:)&]
[s0; Hopefully something real tomorrow...&]
[s0; This was very silly post.&]
[s3; 19.5.2017&]
[s0; Finally something real. The candle`-stick graph drawer works 
now. There is still many problems left even before indicators 
can be drawn. This is definitely the most difficult thing that 
I have ever programmed. That prioritizer code is a b`*tch. The 
progressing speed is about 1/5 of what I expected. Not fun.&]
[s0; [/ Next stop]: adding connections of multiple symbols and timeframes 
to a single core`-object at the single job`-queue iteration in 
Prioritizer`::RefreshJobQueue. That kind of stuff needs a new 
morning and a fresh cup of coffee.&]
[s3; 20.5.2017&]
[s0; I added all`-symbols`-enabled and all`-timeframes`-enabled bits 
to the combination, which allows some custom`-core`-objects to 
automatically use multiple symbol sources. That caused huge job`-queue 
to be created, which was correct, however. In this version, the 
core`-object is allocated to the job`-item, but with all those 
jobs, it takes too long time to allocate them all, and it seems 
to use too much memory in total. So, clearly this was a flaw 
in the design. It will probably be fixed by allocating only necessary 
core`-objects in the processing phase.&]
[s3; 23.5.2017&]
[s0; Today, I worked on the feature that allows user to configure 
custom core`-objects, e.g. changing period in the moving average 
indicator. This version, which uses prioritizer to resolve dependencies 
for the export object, doesn`'t allow configuring dependencies 
from dependee. It causes some headache, but it is actually very 
reasonable feature. Turns out, that most of the indicators doesn`'t 
require features, that couldn`'t be calculated locally, and those 
shouldn`'t be in the way of the prioritizer combination. Rest 
of old indicators, which relies heavily on some dependency, usually 
just proxies argument values to the dependency, and those values 
could be written directly the dependency. ... . I didn`'t foresee 
these things, but they occured while I was trying to implement 
some required features.&]
[s0; Also, some other simplifying was done today. [/ GetIO ]has to 
be changed to [/ IO,] because it can now be used for setting values. 
These simplifications makes me happy, because I don`'t need to 
reduce features, the amount of code is less and the readiblity 
of the code is better.&]
[s0; I still don`'t have a fast hand`-routine for converting old 
indicators to the current system, but it looks like in following 
days it could be found. I still have a problem, that some custom`-core`-classes 
requires different symbols and timeframes, and those should be 
known partially in the combination creation and entirely in the 
custom slot creation point, which is still long before the creation 
of the object... So, I guess there must be some sort of lambda`-function, 
that can be statically exported to change those arguments.&]
[s0; To sum it all, in the old version, all dependencies could be 
configured in the Init method, but in the current version, all 
custom`-class dependencies must be solved internally, and all 
different symbol/timeframe dependencies must be solved with a 
static function. Previously 1 part, now 2 parts. All java programmers 
have been used to static functions and classes, but with the 
C`+`+ rule, that everything must be owned by someone, static 
functions must have a good reason to exist. Of course, some people 
code C`+`+ like java etc. and it works, but with the target of 
clean, readable, safe and compact code, you don`'t code C`+`+ 
like java.&]
[s3; 24.5.2017&]
[s0; Quick update, because I`'ve already programmed enough today.&]
[s0; This prioritizer class is ridiculously complex, but it will 
allow some complex optimizing, which can take some shortcuts 
in the heavy processing, hopefully. Usually those custom`-core 
classes make just clean pipes, but sometimes they require data 
of different symbols and timeframes, and you don`'t want to calculate 
them all. That was the problem, and today I implemented the feature, 
which allows adding different symbols and timeframes to input 
with a static function. This was something, what I didn`'t see 
coming, and I haven`'t programmed anything like this before. 
So, it is challenging, but also interesting. Everything in Prioritizer 
implementation looks very weird and complicated now, even for 
me. I can`'t even split big loops into separate functions, because 
I don`'t see clear pattern. It`'s like programming in assembly 
language.]]