public final class Person {

    private Person spouse;

    /*@
    
    predicate valid(Person spouse) =
        [1/2]this.spouse |-> spouse &*& spouse == null ? [1/2]this.spouse |-> null : [1/2]spouse.spouse |-> this;
    
    @*/
    
    public /*lemma*/ void spouse_symm()
        //@ requires valid(?s) &*& s.valid(?ss);
        //@ ensures valid(s) &*& s.valid(ss) &*& ss == this;
    {
        //@ open valid(s);
        //@ open s.valid(ss);
        //@ close s.valid(ss);
        //@ close valid(s);
    }

    public Person()
        //@ requires true;
        //@ ensures valid(null);
    {
        //@ close valid(null);
    }
    
    public Person getSpouse()
        //@ requires valid(?s);
        //@ ensures valid(s) &*& result == s;
    {
        //@ open valid(s);
        Person result = spouse;
        //@ close valid(s);
        return result;
    }
    
    void marry(Person other)
        //@ requires valid(null) &*& other.valid(null);
        //@ ensures valid(other) &*& other.valid(this);
    {
        //@ open valid(null);
        //@ open other.valid(null);
        spouse = other;
        other.spouse = this;
        //@ close other.valid(this);
        //@ close valid(other);
    }
    
    void divorce()
        //@ requires valid(?other) &*& other.valid(?ss);
        //@ ensures valid(null) &*& other.valid(null) &*& ss == this;
    {
        //@ open valid(other);
        //@ open other.valid(ss);
        spouse.spouse = null;
        spouse = null;
        //@ close other.valid(null);
        //@ close valid(null);
    }

}

class Program {

    public static void main(String[] args)
        //@ requires true;
        //@ ensures true;
    {
        Person a = new Person();
        Person b = new Person();
        a.marry(b);
        b.divorce();
    }

}