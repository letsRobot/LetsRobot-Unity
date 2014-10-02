#ifndef Tokenizer_h
#define Tokenizer_h

#include <string>

template <typename T>
class GenericTokenizer
{
   public:
      GenericTokenizer(const T * input, size_t size) noexcept :
         input(input),
         size(size),
         delimiters(0),
         nDelimiters(0),
         iInput(0),
         lastDelimiter()
      {
         assert(input);
      }

      GenericTokenizer(const T * input, size_t size, const T * delimiters) noexcept :
         input(input),
         size(size),
         delimiters(0),
         nDelimiters(0),
         iInput(0)
      {
         assert(input);
         assert(delimiters);

         SetDelimiters(delimiters);
      }

      void SetDelimiters(const T * delimiters) noexcept
      {
         assert(delimiters);
         assert(*delimiters);

         this->delimiters = delimiters;

         nDelimiters = 0;
         while(*delimiters++)
            nDelimiters++;
      }

      void Skip(size_t nToSkip) noexcept
      {
         iInput  += nToSkip;
      }

      void SkipNextToken() noexcept
      {
         size_t begin;
         size_t end;
         GetNext(begin, end);
      }

      void SkipDelimiters() noexcept
      {
         assert(delimiters);

         while(iInput < size && CurrentIsDelimiter())
            iInput++;
      }

      void GetNext(size_t & begin, size_t & end)
      {
         assert(&begin);
         assert(&end);
         assert(delimiters);

         SkipDelimiters();

         begin = iInput;

         while(iInput < size && !CurrentIsDelimiter())
            iInput++;

         end = iInput;
      }

      std::basic_string<T> GetNext()
      {
         size_t begin;
         size_t end;

         GetNext(begin, end);

         return std::basic_string<T>(input + begin, end - begin);
      }

      size_t GetPosition() const noexcept
      {
         return iInput;
      }

      size_t GetSize() const noexcept
      {
         return size;
      }

      T GetLastDelimiter() const noexcept
      {
         return lastDelimiter;
      }

      bool HasMore() noexcept
      {
         SkipDelimiters();

         return iInput != size;
      }

      bool CurrentIsDelimiter() noexcept
      {
         assert(iInput < size);

         for(size_t i = 0; i < nDelimiters; i++)
            if(input[iInput] == delimiters[i])
            {
               lastDelimiter = delimiters[i];
               return true;
            }

         return false;
      }

   private:
      const T * const input;
      const size_t size;
      const T * delimiters;
      size_t nDelimiters;
      size_t iInput;
      T lastDelimiter;
};

using Tokenizer = GenericTokenizer<char>;

#endif
