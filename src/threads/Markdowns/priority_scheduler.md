# Changes to the base code
## in thread.h
- in `struct thread` we added:
```c
struct lock *waits_for;             
struct list acquired_locks;
int effective_priority; 
```
- `priority` is the initial priority of the thread.
- `effective_priority` is the donated priority of the thread, and this is the used priority for scheduling. (not used in mlfqs)
- `waits_for` is the lock that the thread is waiting for.
- `acquired_locks` is the list of locks that the thread has acquired.
- declaration of `notifyChangeInLockPriority()`;

## in thread.c
- in `thread_create()`
```c
  if (thread_current()->priority < priority)
        thread_yield();
```
- in `thread_unblock()` and `thread_yield()`
```c
  list_insert_ordered instead of list_push_back
```
- added new function : `notifyChangeInLockPriority()` for priority donation
```c
void notifyChangeInLocksPriority(struct thread* t){
    
    if(!list_empty(&(t->acquired_locks))) {
        int maxPriorityInWaiters = (list_entry(list_front(&(t->acquired_locks)), struct lock, lock_position))->lock_priority;
        if (t->priority > maxPriorityInWaiters)
            t->effective_priority = t->priority;
        else
            t->effective_priority= maxPriorityInWaiters;
    }
    else
        t->effective_priority = t->priority;
    updateNestedPriority(t);
}
```
- changed the function `thread_set_priority()`
```c
void thread_set_priority (int new_priority){
    bool yield = false;

    thread_current()->priority = new_priority;
    notifyChangeInLocksPriority(thread_current());

    enum intr_level old_level = intr_disable ();
        if(!list_empty (&ready_list)) {
            struct thread *topThread = list_entry(list_front(&ready_list), struct thread, elem);
            if (new_priority < topThread->effective_priority)
                yield = true;
        }
    intr_set_level (old_level);
    if (yield)
        thread_yield ();
}
```
- in `thread_get_priority()` we made it to return effective_priority
- in `init_thread()` we added the initialization of:
```c
t->effective_priority = t->priority;
t->waits_for = NULL;
list_init(&(t->acquired_locks));
```
allelem changed to list_insert_ordered
- implemented `bool less()` to be used as comparator for `list_insert_ordered`

## in synch.h
in `struct lock` we added:
```c
struct list_elem lock_position
int lock_priority;
```
`lock_position` is used to insert the lock in the list of locks in the thread in an ordered way.
`lock_priority` bt2ol en fi thread el priority bta3to `lock_priority` mestani el-lock da.
## in synch.c
- implemented `bool max_priority()` to be used as comparator for `list_insert_ordered`
- in `sema_down()` we added the priority donation by calling `notifyChangeInLocksPriority()`
- in `sema_up()` we made it to call `thread_yield()` if the thread that is unblocked has a higher priority than the current thread.
Also called `list_remove()` to &t->elem ????
- in `lock_acquire()` we added the following fragment
```c
if (!thread_mlfqs) {
    thread_current()->waits_for = NULL;

    if (!list_empty(&lock->semaphore.waiters)) {
        lock->lock_priority = list_entry(list_front(&lock->semaphore.waiters),
        struct thread, elem)->effective_priority;
    } 
    else 
        lock->lock_priority = PRI_MIN;
    list_insert_ordered(&(lock->holder->acquired_locks), &(lock->lock_position), &compare_locks_priority, NULL);
}
```
- in `lock_release()` we added the following fragment
```c
if (!thread_mlfqs) {
    list_remove(&(lock->lock_position));
    notifyChangeInLocksPriority(lock->holder);
}
```
- implemented `void updateNestedPriority()` a recursive function that updates the priority of the thread and its holder if the thread is waiting for a lock.
```c
void 
updateNestedPriority(struct thread* t){
    if (t->waits_for == NULL)
        return;

    list_remove(&t->elem);
    list_insert_ordered(&t->waits_for->semaphore.waiters, &t->elem, &less, NULL);

    if (t->waits_for->lock_priority < t->effective_priority)
        t->waits_for->lock_priority = t->effective_priority;

    if (t->waits_for->holder != NULL) {
        int maxPriority = list_entry(list_front(&t->waits_for->semaphore.waiters), struct thread, elem)->effective_priority;
        if (t->waits_for->holder->effective_priority >= maxPriority)
            return;
        t->waits_for->holder->effective_priority = maxPriority;
        updateNestedPriority(t->waits_for->holder);
    }
}
```
- implemented `bool cond_priority()` to be used as comparator for `list_insert_ordered`
- in `cond_wait()` we added the following fragment
```c
waiter.priority = thread_current()->priority;
list_insert_ordered(&cond->waiters, &waiter.elem, &cond_priority, NULL);
```