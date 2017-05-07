topic "Journal";
[#phR $$0,0#00000000000000000000000000000000:Default]
[=phN1;*cR+150 $$1,1#26C05E505BFA12B5D94A958416AE5FF3:Chapter 1]
[=phN11;*R+117 $$2,2#F98403FAA9BE0040ACA893F21E4D9F2D:Chapter 2]
[=phN111;*R $$3,3#0680C0FF67EEE4E167183D1E74BF079F:Chapter 3]
[{_}%FI-FI 
[s1; Journal&]
[s0; This journal is totally ridiculous. It is a casual journal about 
the progress of the Overlook. Please note that my native language 
is not English, but Finnish. This journal is intended for the 
audience who see some entertainment value in this. This shouldn`'t 
be considered to be final statement of anything.&]
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
[s0; ]]