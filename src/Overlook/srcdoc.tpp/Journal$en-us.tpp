topic "Journal";
[#phR $$0,0#00000000000000000000000000000000:Default]
[=phN1;*cR+150 $$1,1#26C05E505BFA12B5D94A958416AE5FF3:Chapter 1]
[=phN11;*R+117 $$2,2#F98403FAA9BE0040ACA893F21E4D9F2D:Chapter 2]
[=phN111;*R $$3,3#0680C0FF67EEE4E167183D1E74BF079F:Chapter 3]
[{_}%FI-FI 
[s1; Journal&]
[s0; I write this journal to tell how original this application is 
and to communicate the countless hours and the sweat what I did 
see for this. The public release is merely a by`-product. The 
progress of this project is driven only by my curiosity. I also 
want to encourage other programmers to write the progress in 
this way. This is not something I would do in all projects.&]
[s0; I write as a casual narrator about the progress of the Overlook. 
Please note that my native language is not English, but Finnish. 
This journal is intended for the audience who see some entertainment 
value in this. This shouldn`'t be considered to be final statement 
about anything. This is not intended to be interpreted in any 
specific way and all interpretations are welcome to a civil discussion. 
The story and names are being limited to include the copyright 
holder only, unless the person has given a permission to use 
his story and/or name publicly. However, the story may include 
a one`-sided and personally experienced story from social events, 
which may or may not be equal to the objective truth.&]
[s0; This document is an artistic expression as whole without target 
audience. This is only an opinion of the author and this is not 
checked for wrong facts.&]
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
language.&]
[s3; 26.5.2017&]
[s0; Today, I am going to rant a little bit. Very little. That amount, 
what you would tell to your co`-workers at the dinner table.&]
[s0; Living in this kind of small and isolated place while supporting 
hi`-tech culture is a disaster. You can`'t focus to essential 
and important details, while everyone has lost their curiosity 
and don`'t see the point in any voluntary team`-work. The only 
motivation is money, and even with that you can`'t get motivated 
and skilled team from this small amount of people. Resources 
are really scarce. Also, the incredibly stupid teenage drinking 
alcoholism culture doesn`'t help at all. It helps low reproduction 
statistics, but it`'s kinda evil way to improve that. So, the 
money loses some attractiveness, if you will still certainly 
get bad jobs and bad team with it. My recent experiences in team 
projects have only supported my understanding. This people is 
just keeping a screen of skilled workforce, they will never be 
capable to do development with this training. I am a skilled 
long`-term hobbyist programmer, and I have sometimes being doing 
[* all] the work in a team, while others are still taking the credit. 
Disgusting. Their moral doesn`'t prevent taking the credit from 
work of others at all. It is very important to offically upkeep 
a image of high`-skilled workforce for economic reasons, but 
in practice, this is just a dog`-training and indocrination.&]
[s0; The public schooling has absolutely nothing to do with my programming 
skills. In contrast, they were actually harming that, because 
they tried to make me do things in old way when I could do something 
more efficiently by programming. So, no thanks to public school, 
at all, or to university, in this regard. Only negative feedback 
for them. For my programming skills, I can only thank: Clickteam 
for The Games Factory, Adobe for Flash MX (now Animate), Microsoft 
for Visual Basic 6, people behind Python, and the biggest thanks 
goes to Ultimate`+`+, which made programming great again.&]
[s0; I am not saying, that money is not a good motivation, but I 
am saying, that you should not start from fast and high yielding 
plan. Being too greedy is bad. Waiting for long`-term reward 
is usually good with a good plan. With that greedy attitude you 
are only producing that marketing crap, which doesn`'t have any 
content in it. If I had wanted, I could have gone making a career 
in software development and have a decent salary in this point. 
I am a skilled programmer, I`'ve been in team leader training 
in army and I have high work morale, so I could have that job, 
but I don`'t want, at least yet. So, I could have had in this 
point a nice career, a big house bought with loan, and big`-tit 
blonde making me beautiful babies, but that was not what I wanted. 
I never wanted that. Ok, so in this point I probably lose all 
US readers :D .... but bear with me, I am living in isolated place 
with 100k`-200k residents, and I am basically writing my name 
to the gravestone if I am taking a girlfriend here. Also, the 
feminist movement, or something like that, have caused high divorce 
rate, and we have more single`-mothers now than ever. So there 
is high probability, that the woman leaves you for the most stupidest 
reason, and ruins everything. Last thing what I would want after 
making a family, was a woman blackmailing me for more money to 
keep the family together. But that`'s the reality here. In Iceland, 
they have the free`-sex culture, what you can search from google, 
but we have the opposite of that. We have some very old`-school 
religious culture here. That, were you are not allowed have TV, 
or even be friends with regular people. Even the regular people 
is rather old`-school in many of those matters.&]
[s0;%- [%FI-FI I tried long`-term relationship once. It was a huge 
failure and caused huge losses for me. The lesson was that money 
is more involved in relationship that people accepts publicly, 
even when the other person is denying the importance of money. 
In my relationship`-story, the first bad sign was that there 
was alcholism in her family tree. At the beginning it was sweet 
and rather normal. She had two personalities. Other was the innocent 
little girl and the other was underage drinking and smoking social 
daredevil. I did fit into the innocent family part well, but 
as her dark side took over her, being her felt like it was forced. 
Look, I am not a morality preacher, but that included constant 
lying and keeping two personalities. Not healthy or fun for others. 
Last part in the relationship was when she found some guy, whose 
family was rich, and who started to give her all sorts of crazy 
pills for free. I guess there was sex involved too. That was 
not a fair competition and I was dragged into that because of 
her dark side. It was way too much for me, but that guy`'s crazy 
pills made her so crazy, that she lost it totally. When I tried 
to break up, she went totally suicidal. She climbed on top of 
high places and tried suicide, and I had to forcily prevent her. 
She even went tried to freeze herself to death by going almost 
frozen lake, and I had to get her out from it. Her alcoholism 
supporting family almost did not care at all, so I was clueless 
what I had to do. ][%FI-FI/ Luckily][%FI-FI  she tried suicide in 
front of her family, so they took her into the mental hospital. 
I thought I was free from that hell, but I was wrong. That girl 
found a guy from mental hospital, who was a huge drug dealer, 
and who made his business all over country. It included stealing 
cars and travelling with them to places (which is in record after 
they got caught). I thought I was free from their idiotic network, 
but unfortunately I was wrong. The girl went even more nuts, 
and in her head she ][%FI-FI/ changed identity with me][%FI-FI  , 
and tried to say to anyone that I did force her in all sorts 
of things while we had the relationship. Doctors and other professionals 
can see past her talk, but her new friends were these incredibly 
retard drug dealer people who couldn`'t. So, she got drunk and 
high and told all crazy things about me being evil monster to 
that people. I got some hint of that, when I was out in the city 
center, and got attacked by 3 guys. They attacked me directly 
from behind, with the only intention to hurt me. They started 
by hitting me with a beer bottle to the head, and I almost lost 
eyesight from right eye. Luckily, I managed to run away with 
bleeding head. I was drunk and saw those guys for 1`-3 seconds 
in dark night and I initally reported them to be something different, 
but afterwards I am sure they were her new friends. At that point 
I started to get treated bad by my old friends too. Turned out, 
that her crazy gossipping has reached them too, and no`-one from 
my old friends had guts to warn me about those empty stories. 
This is small and boring place where stories travel fast. Unluckily, 
even at that point I was far from it being over. In our country, 
the military service is mandatory. I did not support or resist 
the idea and I was somewhat curious about it. So there I was, 
doing it all, you know, in the army. Greetings to all servicemen. 
I should have known, that I had went through traumatic experience, 
and that was not a good place for me, but unfortunately I had 
no`-one to discuss it with. Things went south really fast, when 
I was assigned to a military police team leader team with one 
of her new friends, which was some incredibly retarted and narsistic 
taxi driver. Even at that point I did not know how bad her crazy 
gossipping was. So, that guy had some narsistic personality disorder, 
just like her, and he continued that gossipping in our team. 
Unfortunately, even our ]second lieutenant got that gossip, and 
did not tell me, but took part in the crazy gossip ring. It is 
important to emphasize the point, that second lieutenant military 
policy took part into that gossip instead of solving and eliminating 
it, effectively continuing the violence, which started in that 
attack in the city centre. I was basically bullied by higher 
ranking officials for 6 months, without knowing the reason, and 
afterwards shockingly know that they had taken the side of those 
attackers. I changed to sivil service afterwards, so now I don`'t 
need to be in contact with those traitors of justice anymore. 
After the service I was back in the city. Here the weird treating 
of me continued, and I smelled the injustice, but couldn`'t quite 
grasp it. Basically, her crazy group had got larger and those 
tried to make me look bad and they somewhat succeeded in it, 
but it doesn`'t really matter anymore. Last point was, when her 
newest boyfriend sent text messages to me pretending to be her. 
I met him instead of her, and he repeated death`-threats to me 
and he threated to come into my apartment beating me to death. 
I wasn`'t really in danger, because you know, I was trained to 
be military police and he was skinny and alone, but he might 
have knife or gun, so I did not take risks. Her friends are known 
to have guns also, even though guns are highly regulated in our 
country. Turns out, that bad guys get the guns anyway. The craziest 
part was, that her parents had allowed that guy to go `"make 
me man`" like that. They even insulted my mother, when she insisted 
to be in contact with her parents, because my mother thought 
that there was some sense in them left. Finally it was over. 
I have enough material to make a report, but this is so big mess 
now, that I am not sure will they take me seriously now. There 
is a huge list of unlucky details for that story, but I think 
this is enough now. This might even be too much to tell publicly, 
but I don`'t think this text will reach people very fast. In 
the end, I have heightened sense of injustice and sense of all 
history falsification. I have still constant pain from that attack, 
but luckily I kept my eyesight. I am now greatly tramatized, 
but also I feel the peace with the world now more than ever. 
It is going to be fine. That sad little people can`'t win in 
the long run. Justice is like a weed, finding it`'s way trough 
thoughest cracks in the pavement.&]
[s0;%- I could go on and on, but this was off`-topic, and I have 
work to do, so let`'s focus on that, shall we?&]
[s0;%- ...&]
[s0;%- One interesting feature, what separates Overlook from MT4, 
is having fixed time`-positions, which allows to leave out close`-price 
and time slots. There was a common mistake in many mt4 expert 
advisors, which uses close price even though the decision was 
done in the open price point. Basically, all MT4 code prefers 
close`-price, even though open price would be correct in the 
point of decision. The indicator is recalculated when new time 
bar is added, where the open price is only constant value. So, 
in Overlook there is still possibility to use future data, but 
today I added checks that no wrong indicator can read those. 
It caused errors and solving those actually changed the output 
of some indicators. However, these AI agents etc. won`'t be too 
strict about those things. It is still difficult to keep indicators 
from peeking future and refreshing values correctly when time 
goes by. ZigZag, for example, is completely peeking indicator 
and it is used in the feature detector. It is a huge problem, 
because it gives peeked data. It is currently broken in that 
way, but I don`'t know how to fix it yet.&]
[s3;%- 30.5.2017&]
[s0;%- I just completed the first and simplest query`-table forecaster. 
I had to learn a lot about the decision tree theory via documentation 
and trial and error, but finally I made it. There is many problems 
because of the overfitting, but in this case the underfitting 
was even a bigger problem. Unlike in MT4, in this weekends and 
closed markets causes zero`-change bars, which caused the pruning 
function to delete all nodes which predicted something else than 
zero change. Well, I guess it`'s overfitting to zero`-change, 
but underfitting describes it better.&]
[s0;%- The proper testing is still undone. It probably needs to be 
done in larger scale, with all other classes in the pipeline, 
so I postpone that temporarily.&]
[s0;%- What next? I still have time today. Let`'s see after lunch 
break.&]
[s3;%- 31.5.2017&]
[s0;%- Hello. It starts to look clearer and clearer that neural networks 
are unnecessary for Overlook. Slower timeframes are easier to 
forecast and faster timeframes goes along with the slower usually, 
so you can try to forecast faster changes by using slower forecast 
as input. Events are also highly repeating and networked. I will 
re`-introduce neural networks to this by stricly comparing them 
to decision trees, but current neural network custom core classes 
are wrong. They are not being used and they will be removed. 
Mona neural network is probably gone for good from the Overlook, 
and recurrent networks can`'t do decision tree`'s job, but fully`-connected 
deep neural networks will be used if they perform better than 
decision trees. The nature of the markets is more web`-like than 
reoccuring patterns of single stocks, indices or currencypairs. 
This is simply an improvement in the plan and makes the result 
better, in the exchange to small amount of lost work by me. I 
have no personal attachments to any part of the plan.&]
[s0;%- I feel that doing this project as the public open source project 
helps me to notice obvious things, that other programmers would 
see if they looked my code, even though I can see from statistics 
that nobody probably haven`'t read this yet. Only the notion 
of potential observers seems to make my subconscious attitude 
little different, like what observer does to the probability 
distribution in those weird quantum state phenomenons, but I 
really don`'t know and connecting those two separate ideas is 
silly.&]
[s0;%- I debugged the huge query`-table forecaster. It starts in 
`~5 seconds in my 4GHz machine and seems to give correct results. 
Making the decision tree from this many predictors is slow, but 
it`'s nothing compared to neural network training.&]
[s3;%- 1.6.2017&]
[s0;%- It`'s 1.6, first day of summer, and there is hailstorm outside. 
It looks like it`'s snowing, but in fast`-forward, because hails 
are heavier and comes down faster. The temperature is `+5 Celsius. 
Worst spring in years, as far as I remember.&]
[s0;%- I am re`-planning forecaster and agent classes again. Now, 
when neural networks will compete first with the decision tree, 
there won`'t be much complexity in the pipeline. One agent is 
enough in the beginning, instead of many weird neural network 
efforts. I am using more generic approach now. All classes must 
try to use their slower timeframe instance as input, and they 
must try to work in the fastest timeframe, in which they are 
the proxy of slower tf or they use their own calculations if 
it`'s better. It seems complicated now, but the result should 
be simple. I did something similar with the SubTf and MetaTf 
indicators, which tracked the `"progress`" of expected volatily 
in comparison to longer period. Those indicators are useless 
now, but they were the precursor of this model. I should make 
them work again for historical reasons only.&]
[s0;%- Looks like I get better performance comparison to my master`'s 
thesis now. The description is much simpler now: everything was 
designed with the minimum system (query`-table and decision trees) 
and neural network systems are being compared to the decision 
tree system in 8 different predictor sets. I knew I would get 
some kind of performance comparison, but this is just much more 
smoother and better than I expected. Only the math is an obstacle 
to the completion of my master`'s degree in computer science 
(or whatever it was called). There is just this evil elitist 
math teacher, which is blocking the progress. I have been in 
very different schools in different `"social classes`". I got 
very high grades in the large `"people`'s school`" `-like place. 
There I was [/ the best] in math. Then we moved and I went to [/ the 
elitist school], where my friends behaved like little children 
and they wanted to be doctors and pilots and yes, they became 
those. There they didn`'t even bother to teach. They just kept 
the limit too high to motivate them to do something impossible. 
That was so ugly that I [/ wanted] to change to more liberal school, 
where people didn`'t even bother to try to do the math. Now in 
university, I have only those [/ elitist] and entitlement teachers 
left, who keeps the price of the title high. Basically they exploit 
the fact that there is no real comptetition to make good learning 
material in good price, and they can just throw the bone to the 
most difficult maze and say `"[/ go fetch it]`", instead of providing 
comprehensive material with included solutions, so that they 
could be analyzed and the whole structure of things could be 
observed and solving problems could be optimized. It`'s like 
there is a artifical limit to prevent people learning more in 
the sake of their elitist social entitlement program. What I 
have done with my computer science attitude, is that I searched 
the automation to all math problems, including statistics and 
differential equations, which made me also to understand the 
math behind it all. However, [/ the evil teacher] actually did 
put to exam [_ only problems without automated solution] (all exception 
cases)[* ,] and he continues to do it, essentially stripping off 
all my motivation. So basically he does huge effort to prevent 
this method to work and he tries to make old`-school math heads 
look better than they deserve. So, I am stuck with that old`-school 
devil who actually actively targets my method to prevent it to 
work. He is literally so bad, that his statistical exercises 
includes proving, that `"[/ punishing children with violence improves 
their learning]`". It`'s fucking unbelieveable to have that kind 
of teacher in an university and this doesn`'t even come from 
some [/ `"leftist safe`-space student`"]. It`'s really medieval.&]
[s3;%- 3.6.2017&]
[s0;%- Hi, again.&]
[s0;%- This is the day when I thought that this program would have 
been ready. I hope that two week extension is enough, because 
I have huge todo list of school work for this summer. I couldn`'t 
get any summer job because of that, which makes the budget tight, 
but bearable.&]
[s0;%- There have been some change of plans already today. I started 
normally by planning the implementation of the `'[/ Custom]`' forecaster 
class, which was next thing in the TODO list. Then, I thought 
that some more abstract Model class would be better and it could 
be reused by inheriting in other forecaster classes too. That 
required checking of all requirements for upcoming forecasters, 
and that required the detailed description of every class. After 
describing classes the Occam`'s razor principle did hit me once 
again. One class might be better than all of those custom classes 
[/ (one class to rule them all... is the Lord of the Ring only 
about Occam`'s razor principle after all?)]. So, instead of inheritable 
Model class, I decided to do only one class. That is a good change 
of plan, but all change of plans tends to cause much stress for 
me because of some [/ biological branch predictor in the brain] 
or whatever. I can probably plan the class today only, and that`'s 
all.&]
[s0;%- ...&]
[s0;%- I have planned again big portion of the app. This follows 
the Occam`'s razor princple. This is smaller, scales better and 
the previous version is a subset of this. As usual, some work 
seems to have been useless. In previous plan, I thought that 
I could measure the performance of pipelines and then construct 
better pipelines with that information. My mistake was, that 
I thought that it could be performed better with decision trees 
and NNs than with genetic optimizer. I was wrong. Clearly some 
result table with genetic optimization is better. You know, sorting 
results by performance, combining best ones, adding random values... 
With that, only the traditional use case is problematic with 
candlesticks and an indicator of single sym/tf combination. &]
[s0;%- ...&]
[s0;%- I need to make big changes to the whole project structure. 
I broke some code and I will fix it in next days. I did the detailed 
list of changes but I don`'t care enough to translate it to English 
and add here. The revised plan includes everything that current 
version has and solves all remaining issues with the pipeline 
optimizer. Now I can`'t say anymore that I had done useless work 
at all, because Prioritizer clearly did solve many other problems 
and it will be re`-used as parts. Implementing the new System 
class will cause a lot of headache, however, and I am not happy 
about it.&]
[s0;%- I think Overlook will be ready in 2 weeks.&]
[s3;%- 4.6.2017&]
[s0;%- I can easily say that the [C SystemOptimizer.cpp] and it`'s 
past versions is the most difficult thing I have ever programmed. 
It`'s one of those things where you know parts, but not what 
the whole combined thing should be. Basically it genetically 
optimizes tree`-structured pipelines (with differential evolution 
algorithm) and creates job`-queues where all duplicate objects 
are avoided for performance reasons. All leaf objects can be 
easily re`-used and the root`-object is never the same, and the 
area in between is like gradient of those. Object dependencies 
and their priorities must also be known, so that less important 
objects can be released instead of regularly used objects. Now 
I am trying to re`-use existing code from Prioritizer, but the 
task is difficult. You can usually do a lot of routine programming 
or limited amount of problem`-solving`-like programming in a 
day. The [C SystemOptimizer.cpp] is a huge pile of problems, which 
demands many days, while in the paper it looks like there is 
almost no progress at all. The plan is detailed enough, but it 
also must be simplified at the same time when parts are connected 
and unnecessary complexity is recognized, which makes this a 
constant battle with heuristic guide. That kind of working is 
hard to do in a team and zero`-budget... Luckily, this, the most 
frustrating project I`'ve ever worked with, is probably finished 
in about two weeks.&]
[s3;%- 5.6.2017&]
[s0;%- I am still working on [C SystemOptimizer.cpp] and it still is 
ridiculously difficult. I had to write all certain things as 
comments in that file. That showed the point of uncertainty very 
well and it was like modern highway stopped suddenly to the wall 
of rainforest jungle. There is some code from old Prioritizer 
class, which implements all the same necessary stuff but for 
different system, which is like a lighthouse glowing in the dark, 
but it doesn`'t help to get trough.&]
[s0;%- ...&]
[s0;%- What... I made huge mistake again... I thought that a type 
of input`-indicator could be in one column, but I thought it 
and it is bad idea. The mutation happens by combining many values 
and it doesn`'t work for classifiers. I knew it was wrong from 
past coding, but I didn`'t connect it to this. Mutating the enabled/disabled 
bit works, however. That was how it worked in the Prioritizer, 
but somehow I missed the point once again. So, now I must re`-check 
all column values again, that there is not a single classifier 
as a value. Only weaks classifiers are allowed, like which average 
method is used in moving average...&]
[s0;%- Hmm.... seems like I am so stuck with this that I didn`'t 
even write it in a wrong way yet.&]
[s0;%- So, now there must be room for enabled bits for all traditional 
sources, and their arguments, to make the mutations work. That 
shrinks the tree`-structure of template classes to a stub, but 
that doesn`'t really matter. I didn`'t expect the multi`-level 
structure to bring much more value anyway.&]
[s3;%- 6.6.2017&]
[s0;%- I was so pissed yesterday about being stuck, that I didn`'t 
commit anything. The way I commit everyday, is not good for community 
or company projects, of course, but this is not one. I have been 
in team projects where I was the only one to make commits and 
the work anyway, so I got less interested about if the project 
will compile and link... &]
[s0;%- ...&]
[s0;%- Exciting findings... I simplified some old code and that gave 
me some interesting questions and answers...&]
[s0;%- So, the decision tree allows only one target value. At first, 
you think that it should give the long/short signal. `"[/ But but 
but I want more than one investment instrument...]`". Ok, fine, 
either you discard the decision tree or you start to be creative. 
Why don`'t you just use artifical neural networks, which gives 
you multiple outputs, which you can interpret as many long/shorts 
signals and give reward accordingly? Well, remember how I told 
how there is this tree`-structured pipeline, which has combiner 
at the root and the disabled/enabled bit is for leaf nodes to 
allow sane mutations in genetic optimizer? Turns out, that it 
might not be that good idea to put enabled/disabled bit row to 
the direction of root, like it is in the direction of leafs... 
That would be something that would happen if you would put artifical 
neural network in the root combiner without working decision 
tree model to replace it.&]
[s0;%- There is a good reason for not using that enabled/disabled 
bit row for all market symbols. If you look what the correlation 
oscillator indicator draws, you`'ll see, that there is usually 
very strong correlation for or against other symbols. You`'ll 
see, that sometimes some symbol suddenly flips it`'s side (`+1 
to `-1 or `-1 to `+1), but most of the time symbols are almost 
`-1 or `+1 by their correlation value (with medium to small sample 
size). There is actually a good classifier for root target which 
has a reasonable description. That would be [/ strongly correlating 
groups with descending order]; the largest strongly correlating 
group would have the identification number 0. Weaker groups would 
have their id starting from 1. After that there would be sub`-groups 
and maybe even some intersecting areas. I think that is what 
the Overlook will have as the target value of the root combiner. 
However, I might find some flaws in this model.&]
[s3;%- 7.6.2017&]
[s0;%- Good morning. I woke early enough to watch sunrise from clear 
blue sky at 4:30 am. I am lucky to have very nice view for that 
from my desktop where I program. It`'s going to be warm (20 Celcius) 
 day. In these days trees will get their full leaf and the summer 
will truly begin. Best time to be in Oulu, Finland. It`'s already 
entirely bright night and it continues to be so for couple of 
months while having the peak brightness after two weeks from 
now.&]
[s0;%- I noticed, that I should create virtual nodes for baskets, 
just like I have already done for single currencies now. In that 
way there is no need to calculate values for items in basket 
separately. In those baskets I would put highly positively correlating 
symbols, which is opposite of hedging, where you put negatively 
correlating symbols in the same basket to cancel changes in total 
value.&]
[s0;%- ...&]
[s0;%- A lot of progress was made today with SystemOptimizer.cpp, 
but it`'s still not completely in the previous working state. 
It`'s worth noticing how much complexity have been reduced today 
without limiting features.&]
[s3;%- 8.6.2017&]
[s0;%- The perfection is forever, and our brains and genetics only 
tune into that eventually. There, I said it. That`'s how I believe. 
Sooner than later is usually more pleasant, but rush too fast 
and you might not see something important.&]
[s0;%- To the practical matters... The classic viewing mode works 
again (candlesticks `+ indicators). I have had this at this point 
many times before, but I had to change something structural, 
so something would break and I would have to fix it.&]
[s0;%- About the basket symbols what I mentioned yesterday... changing 
between those at different times should be decided by the optimizer, 
not by some template object. That would take the planned combiner 
object away, and that is probably how I end up doing. It will 
decrease a great deal of complexity, because there is no need 
for combiner to listen all the most important baskets and switch 
between them. Optimizer will have the primary time`-range, which 
will be trimmed to include the most efficient time`-range, and 
secondary time`-slots will fill the remaining time, and they 
will be trimmed again until limit is reached. This kind of simplifying 
is very powerful and this is how you should do it: immediately 
after you have recognized the opportunity and before the old 
and more complex design has been implemented further. Unfortunately, 
sometimes you have contracts to make something in a bad and old 
design. There is, of course, a small probability, that some opportunities 
turns about to be useless learning experiences, meaning that 
you were wrong ;)&]
[s0;%- ...&]
[s0;%- I took a walk (there`'s freakin`' hot outside) and got an 
idea, that maybe the optimizer should seek to the timeslot `& 
basket instead of only timeslot. Also some basic environment 
states could be seeked. The optimizer becomes some multi`-headed 
monster, which tries to fill the whole surface with some sharpest 
tops. Every head has a differential solver and a decision tree 
pruner and they have a strict priority order. That seems interesting, 
at least for me.&]
[s3;%- 9.6.2017&]
[s0;%- Ok, so the simple and efficient solution is to add timeslot 
and basket optimization to the differential evolution solver. 
No need to make it more complicated.&]
[s0;%- The main optimization loop is slowly but steadily starting... 
Next: basket bardata from long argument vector and templates. 
The alpha version will be ready soon.... I can`'t wait to get 
this project to slower development phase.&]
[s3;%- 10.6.2017&]
[s0;%- Happy name day me.&]
[s0;%- One can connect dots, how the Overlook is using MT4 and market 
symbols and genetic optimization to improve results, but one 
important feature of the Overlook is to make connecting those 
dots difficult. I mean, you can`'t give everything to everyone 
on the golden serving dish, right? However, the life doesn`'t 
need to be bloody zero`-sum game neither. Some golden rules must 
be applied.&]
[s0;%- I have witnessed the most heart breaking discrimination of 
poor people many times, which have had completely elitist zero`-sum 
game nature. The most heart breaking is that, when someone haven`'t 
had a chance yet, and the weaker position have been the fault 
of their parents. Of course, we have genes, but we also have 
brains, and it`'s weak minded to weight bad genes or bad inheritance 
instead of strong. My sister is one of those. Girls are very 
evil in that point. They demand irrational sweetness of life 
more than men, and discriminate other girls for the stupidest 
elitist reason. Men at least can come together with the sports 
by nature. It is also sad to see accumulating effects of one 
single unjust treatment and the evil ignorance of those greedy 
sweet`-tooths. That makes you want to listen to `"The Prodigy 
`- Smack My Bitch Up`".&]
[s0;%- Ultimately, there is the world of all uncesessities, which 
allows using human resources by luring people unknowingly into 
life`-long traps with marketing and claims for moar success, 
moar `$`$`$ and moar something, which is generally accepted for 
some GDP increase or whatever. Start looking that more in`-depth 
and someone will say `"[/ The big bubble of human resources filling 
the result of evolution of the society is none of your business, 
stay away!]`", `"[/ No taking of pictures!]`" or `"[/ Only the simulation 
of official timeline in very low resolution for recreational 
purposes is allowed!]`". However, the brain`-dead culture of 
recreationial ceremonies and related stories is only valid as 
long as those stories are not analyzed with personal computers 
using scientific big data methods, artifical neural networks 
and logical reasoning. Even then they would try to sell some 
of it as true by some so`-called trusted scheme and religious 
fake`-science. Only the immature system needs the exploration 
of doing something which is not needed, but it is hard to tell, 
which parts of the system actually are mature. Some parts have 
known mature versions, but some narcissistic predatory adult 
cry`-babies are blocking their distribution, because it would 
prevent them exploiting existing immature versions. The corruption 
is about someone trying to keep your life limited to the noisy 
M1 timeframe and putting random expensive nonsense into it. It 
is about making you sad but smiling under eye. It is about making 
someone else too rich in your expense without you knowing it. 
It is about not having opportunities to travel and meet new people 
by making you to scare your own shadow.&]
[s3;%- 11.6.2017&]
[s0;%- Today, I want to talk about porn. I have been working on the 
DataBridge correlation code, but it doesn`'t have anything to 
mention about.&]
[s0;%- Oh porn, the magical thing which tickles our primitive instincts...&]
[s0;%- I was one of those who found the secret porn stash with my 
4 friends. That was not my father`'s but my friend`'s father`'s. 
I was around 10 years old. We didn`'t think it as bad then, and 
I haven`'t changed my mind ever after. I understand the sensitivity 
of the subject, however.&]
[s0;%- Few decades ago, everything was more innocent. Pinup girls 
showed only a little bit of leg and had their mouths shaped in 
a weird kissing way. Then the actual penetration came along. 
It was tied to a story, but eventually the story had to go. Then 
came renting of videos and Internet. The XXX culture started 
to grow even more as business.&]
[s0;%- All of those of us who have seen porn knows, that there is 
a lot of dick and pussy in the screen. You can`'t somehow unsee 
men`'s parts even if you are heterosexual, like me. It is part 
of the whole product. You start to understand the friendship 
between straights and gays. There is no big difference between 
what you see with girl`-boy anal and what gays see with boy`-boy 
anal. You don`'t have the feeling, but you can understand.&]
[s0;%- I like mine in traditional ways. Big tit womanizers are a 
huge seller for me. However, that is not very popular anymore. 
The statistics shows, that most popular videos are not about 
the image of women what 70`'s James Bond films gave. In view 
statistics, it is going to weird direction... Most of the society 
is completely clueless about the real evolution of adult entertainment. 
Some may have noticed, that sex toys and lingerie is more popular 
than ever, but that is not the whole story.&]
[s0;%- This topic deserves a scientific paper, but unfortunately 
I have time to write only about my experiences. It is extremely 
important to understand, that the virtual adult entertainment 
industry has nothing to do with non`-virtual counterparts. To 
my understanding, only a small fraction of audience would mix 
those two. Healthy people will have strong emotional response 
to physical contact, which is not comparable to virtual experience.&]
[s0;%- Mixing the pleasant virtual experience to real life can be 
challenging. Many of us are in a relationship, where the partner 
have had a traditional and religious image of reproduction. Someone 
can sometimes find out, that the partner has insanely strong 
opposing reaction to porn. The topic may scar both the porn loving 
and the porn hating party. Wrong behavior is to prevent the other 
loving porn or trying the other to love porn. Personally, I was 
one of those unlucky men, who had that kind of partner, which 
tried to both prevent me loving porn and who accused me trying 
to make her love porn, when I obviously rather broke up with 
her for being selfish and narcissist. She was a actual nutjob 
in other ways too, though. She went to extreme opposition and 
started to ruin my public image with insane lies; after I had 
saved her life around 5 times from violent suicide and was crying 
from emotional stress, she was still in denial of me being the 
good guy so much, that she started to gossip that I was weird 
crying gay`-hater. My biggest mistake was, that I didn`'t file 
a official report, but I was young and clueless and adults around 
me were stupid. Shit happens.&]
[s0;%- Anyway, for some of us the porn is a healer, which keeps us 
sane in the hateful world, and for some others, it is from the 
devil which eats good men. Pick your side.&]
[s3;%- 12.6.2017&]
[s0;%- Focusing on the project again. Basket symbols are now working 
and the next step is to make the optimization loop running. After 
that is the implementation of Template class, all the detailed 
debugging of optimization and real`-time code. Finally there 
is only cleaning of code, commenting and documentation. This 
took longer than expected, but this should be over in a couple 
of weeks.&]
[s0;%- ...&]
[s0;%- The main optimization loop runs now. It is unstable and template 
doesn`'t process for real, but it runs for a long time until 
some small bug occurs with the traditional indicator.&]
[s3;%- 14.6.2017&]
[s0;%- Today, I had to take a big step backward. There were some 
undesired setbacks with the target value to genetic optimizer 
and tweet from `@karpathy yesterday gave insight to a new method. 
Previously, I have translated the `@karpathy`'s ConvNetJS library 
from JavaScript to C`+`+ and I understand how powerful neural 
networks are, but I always thought that the differential evolution 
solver would be better in this problem. I think I made a mistake 
in that. The link was to [^https`:`/`/blog`.openai`.com`/deep`-reinforcement`-learning`-from`-human`-preferences`/^ L
earning from Human Preferences] and it has a great footnote comparing 
human input and user defined reward function. Trading is also 
complex problem like that. Also, user input to trading is important 
for ethical reasons, me thinks. 100% automated conqueror sounds 
baaaaaad... and technical leverage for user`'s preferences sounds 
like a shoe that fits well.&]
[s0;%- What I understood from that scientific paper, was that it 
is almost like a DQN agent where the program gives a set of visualized 
decisions and users decides which is better. `"All weight is 
put on the better`", which is something I don`'t understand in 
practise. It might be, that the better set is being used in training 
only and worse is discarded, or it might be that the better might 
be trained with `+1 reward and the worse with `-1 loss. The difference 
to DQN is that there is no begin and end states with reward but 
right and wrong set of states.&]
[s0;%- I guess I must go back to basics and do an example with air 
hockey, which is something what I planned to do with mona, but 
this is fine too. I did some useless work, but I also did some 
awesome features what I wouldn`'t otherwise had done, like highly 
correlating basket symbols. I didn`'t trim any features and this 
is simplier, so it can`'t be bad neither.&]
[s0;%- Replacing that differential evolution algorithm with some 
DQN`-like NN causes some new problems. Evolution algorithm works 
fine with huge list of items in the set, but NN slows down fast 
with increasing items. Also, getting multiple output values from 
DQN`-like agents is problematic. I got an idea from Mona NN that 
you could change location and that would be remembered while 
reading sensors, so with this DQN`-like NN it could also work. 
Instead of putting all available sensors to the NN, you could 
change reading position, and then proceed to different actions, 
until wait action is selected. That`'s something worth testing. 
I guess both methods could be compared with that air hockey, 
because it still works with that single action per time`-step 
method.&]
[s0;%- Hmm... it actually has even simpler version, which it sort 
of inherits. You could put those sequences to querytable with 
those weights, and run it from that, instead of training NN. 
Actually, you could skip the user rating which sequence is better 
by simple function.&]
[s0;%- I am trying to follow that paper and start with existing DQN`-agent, 
but I will adjust that to larger amount of inputs by adding sub`-timestep 
`"movement`" and few writable memory`-sensors. Then it is going 
to train the better sequence, which is selected by some simple 
function, and which contains many time`-steps. It is a make`-it`-or`-break`-it 
moment for me at this summer, because I don`'t have much time. 
If it doesn`'t work, then I need to read more some scientific 
papers and try again later. I have only good experiences from 
DQN`-agent, however.&]
[s3;%- 15.6.2017&]
[s0;%- That sequential comparison for DQN`-agent was the missing 
key, what blocked me from using DQN in overlook. I am thankful 
for that paper. I need to remove some existing code, but not 
too much. At least the DE solver will go.&]
[s0;%- The other approach seems to fit unexpectedly well now. It 
is a iterator going trough the time`-range and showing all information 
in a heatmap, and then NN get trained by more correct sequence 
from two sequences of actions. It requires a lot less complexity 
from the system, but it still requires a huge work`-queue to 
be processed until even one single time`-step can be taken. It 
must process all data and technical indicators before starting 
iteration.&]
[s0;%- One interesting new approach would be to sweep the whole data 
range in one time step in fixed order and use next time`-step 
result as the comparison value, and then train the better sequence. 
In the context of exhausting amount of inputs, that might be 
worth testing. Some read/write sensors could be used as memory 
during the sweep.&]
[s0;%- Hmm... seems like this is a possible many`-core application. 
I might have misunderstood some details in that paper, because 
the lack of source code, but currently I understand it as an 
extended DQN`-agent with training on better sequence of two sequences 
and with fixed zero reward. With too many inputs for sensors, 
one could try to sweep all inputs with DQN`-agent having small 
`"register`-memory`" in sensors and sweep it syncronizedly by 
many of those small DQN agents in same spot or different spots. 
They would all write to outputs until time is out for current 
time`-step. Two sequences would be measured and better one would 
be used in training. I don`'t know does it work or is it useful, 
but it would fit well to GPGPU kernels. It would be something 
like [* many`-inputs to many`-outputs with DQN`-agent swarm]. It 
would be very interesting to test that... It would be almost 
like that example in the paper, where the sequence consist of 
multiple time`-steps, but in this steps would be inside one time`-step 
and only one time`-step would be compared, because of the length 
of the sequence. Basically it would shrink multi`-time`-step 
sequence to one time`-step sequence, and use it to control multiple 
inputs and outputs internally. Yeaaaa, I think I`'ll do it immediately. 
I`'ll fall back to that paper version when everything fails...&]
[s3;%- 16.6.2017&]
[s0;%- I`'ve been thinking about that DQN`-agent swarm, and one interesting 
use case to test would be to create huge amount of small agent`-compatible 
objects (aka. physical swarm) and then add 0`-10% of DQN`-swarm`-agents 
for them. Then you would loop those swarm`-agents through all 
objects multiple times in one time`-step and then you would compare 
two outcomes after one`-step, and you would prefer the sequence, 
which is better e.g. where swarm objects has less collisions. 
You might even be able to move huge swarms through some obstacle 
courses, like training some flying bee swarm. If that would work 
in pracise, it would be an ideal many`-core application.&]
[s0;%- Today, I hope I can get the iterator working for that DQN`-agent 
thing and start implementation of the agent too.&]
[s3;%- 17.6.2017&]
[s0;%- It was hard to see the obvious solution somehow. The most 
safest and fastest design is to use one sequence`-DQN`-agent 
per sym/tf and give them few steps inside time`-step to allow 
competition. That can be implemented in Cuda or OpenCL too. Luckily, 
I didn`'t implement any other design yet. This design is almost 
proven with the ConvNetC`+`+ WaterWorld demo and with that scientific 
paper. Only difference is to make agents interact with each other 
for common reward. It should work as expected as the behaviour 
is similar to the WaterWorld demo.&]
[s0;%- I am very happy about this fix. Now the design matches to 
my previous expectations about combining all symbol instances. 
I tried many different suggestions, but this is the best so far. 
Rest of the details will show themself while I implement this.&]
[s0;%- The Overlook is now rapidly approaching the alpha version.&]
[s3;%- 19.6.2017&]
[s0;%- The main trainer loop runs, but without SimBroker implementation. 
I`'m worried that I forgot some sensors and I`'m worried that 
the agent has too many sensors now. Other parts of the loop seems 
to be okay; I can`'t find bugs with random checks.&]
[s0;%- Upcoming: SimBroker implementation and GUI for training, which 
allows inspection and debugging of the training. I`'ve been busy 
and super lazy in past days, but I hope I get better. The good 
objective is to get real`-time trading working before weekend. 
That would also mean that the first alpha version would be ready 
at the midsummer party and I would have two good reasons to take 
beer. The long hangover wouldn`'t be a problem, because following 
development would be on`-demand after test results. Maybe I`'ll 
work harder when I think that as a reward...&]
[s3;%- 20.6.2017&]
[s0;%- This journal tends to go off`-topic too easily... but I think 
this free`-speech smell in this kind of project makes a good 
statement. I only draw a line on political commentary.&]
[s0;%- I`'ve been writing the SimBroker`::Cycle() function and it 
is really difficult. It`'s too overwhelming to write it at once. 
I have written all parts of that previously, but this time all 
parts are in one tight function. I have only this task for today 
and it might still be too much.&]
[s0;%- ...&]
[s0;%- So, it is sort of difficult to progress with the implementation 
now. I couldn`'t plan this better because of unknown release 
target and now when it is known, it is hard to get good picture 
of everything. I think I should literally draw picture of this.&]
[s0;%- ...&]
[s0;%- I remembered what the reward is despite the performance of 
the trading automation; you get constant excitement about being 
involved in the markets with real money. It is like morning coffee. 
That is my [/ real] objective, even if the performance would be 
rather bad. I am also confident that I can get something worth 
processing with the sequential`-DQN`-agent, but it might take 
a while to tune it correctly. This is my 10`-20 year plan, so 
I`'m not in a hurry. I am sharing this program to help others 
to get that kind of excitement, but please don`'t quit your day 
job.&]
[s3;%- 22.6.2017&]
[s0;%- Good day. Seems like I get stuck with this before my morning 
coffee ends (which is medium roast 0.7 litres with milk and without 
sugar by the way, and some would say that it is way too much). 
Also, I have work to do relating to next year in university, 
so I am doing this only at the morning for now on.&]
[s0;%- ...&]
[s0;%- I think the education about AI in investing is totally non`-existent. 
While I highly appreciate the MetaTrader community about their 
efforts, I think the quality of those expert advisors is mostly 
total rubbish. This is what I think about them:&]
[s0;i150;O0;%- They don`'t recognize slower timeframes being usually 
easier to predict and trading faster is exception to the slower 
target.&]
[s0;i150;O0;%- Some instruments follow other: just try some simple 
cross`-symbol predictor and test to hide each of them. It shows 
that hiding the leader increases the error rate.&]
[s0;i150;O0;%- They don`'t try to calculate virtual currency instrument 
from pairs, which could be good source of cross`-symbol prediction.&]
[s0;i150;O0;%- They don`'t have money management usually, and if 
they do have, they don`'t have the safe range optimizer. This 
is the important factor which could enable exponential growth 
in theory (, but probably not in practice).&]
[s0;%- The Overlook considers all of those things while being free 
and open source. You`'re welcome.&]
[s3;%- 23.6.2017&]
[s0;%- Implementing SimBroker`::Cycle turns out to be harder and harder 
every day. It should be done with correct exposure calculations. 
I have correct (enough) exposure calculations, which I modeled 
from exposure`-values, what MT4 gave. However, those work in 
forward propagation way and now I need backward propagation. 
With this, I must start from the value what forward propagation 
returns. Why? Well, when you want to update your orders and want 
to have some target total margin, then you want to get real volumes 
from relative volumes of your requested orders. If you have already 
opened orders, you want to take those into account with their 
existing margins. To make this even more complicated, this is 
called in the most busiest training loop in every time`-tick, 
so it should be high`-performance`-computing`-friendly.&]
[s0;%- Seems like Brokerage class has had too little attention: SimBroker 
and MetaTrader classes has many duplicate features. Also, it 
should cache variables better, because the bridge is sloooooooow.&]
[s3;%- 25.6.2017&]
[s0;%- The Brokerage class is still a mess. Nothing more to say...&]
[s0;%- ...&]
[s0;%- Hmm... it seems like it is difficult to separate pairs from 
virtual currency nodes while calculating volumes from total base 
currency exposure. Also, it is tempting to treat currencies as 
a basket of pairs. However, preferring that would require proving 
the assumption, that virtual currency nodes maintain integrity 
better than other kind of baskets, like basket of highly correlating 
pairs. I actually have very little knowledge of these sorts of 
things, because I can`'t find relevant scientific papers, and 
they might not even exists. This is mostly my best guess only. 
I am lacking all proper theory. However, the objective is to 
get something worth running at background (and this is not rocket 
science), and current reasonable solutions and minimum requirements 
would suggest that currency basket (of pairs) would have higher 
priority, and other sort of baskets should be added later while 
comparing their performance to currency baskets. This description 
is not as clear as it is in the code.&]
[s0;%- In the code, it seems to be reasonable to give relative weights 
for currencies, indices and stock CFDs, and the program would 
solve weights for pairs from currency weights. That would require 
minimum amount of programming while being more intuitive in real`-life. 
The difference would be to not allow giving signals for single 
currency pairs, which is something that manual traders wouldn`'t 
even consider.&]
[s0;%- Basically, the Occam`'s razor principle would suggest that 
manual traders trades only the most direct currency pair, and 
automatic traders trades weighted pairs from currency weights, 
because the correct backward exposure calculation is easier in 
that way (to put it mildly). That doesn`'t even have anything 
to do with the integrity of virtual currency node from pairs, 
but which is also reasonable assumption and which would suggest 
that basket of pairs is more stable and easier to predict. Also, 
single pairs tend to have more spikes, which is also something 
to avoid with automated solution.&]
[s0;%- In the end, benefits for ditching single pair signals and 
custom baskets until further interest outweights my curiosity 
to research this more.&]
[s0;%- To simplify to the maximum: the sequence`-DQN`-agent gives 
direct signal to the asset, and then that drives volumes of instruments 
indirectly using that backward exposure calculation.&]
[s0;%- To be continued tomorrow at the same bat channel `[[^https`:`/`/www`.youtube`.com`/watch`?v`=alQ0zUjLLmg^ 1
], [^https`:`/`/www`.youtube`.com`/watch`?v`=mlsXRq`-Y6zE^ 2], 
[^https`:`/`/www`.youtube`.com`/watch`?v`=iwbsx6LvnfY^ 3]`].&]
[s0;%- ]]